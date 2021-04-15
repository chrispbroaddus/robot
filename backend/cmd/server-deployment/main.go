package main

import (
	"bufio"
	"bytes"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"os/exec"
	"strings"

	arg "github.com/alexflint/go-arg"
	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/endpoints"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/ecs"
	"github.com/aws/aws-sdk-go/service/s3"
	"github.com/zippyai/zippy/backend/configs"
)

var (
	// ErrNotMaster is thrown if a user is not currently on master when running
	ErrNotMaster = errors.New("you're not currently on master, please checkout master before running")
)

func main() {
	var args struct {
		Environment    string
		Manifest       string
		ReleaseVersion string
		CurrentBranch  bool
		NewInfra       bool `arg:"help:Create a new service rather than updating the existing one"`
	}
	arg.MustParse(&args)

	tasks, err := createTaskFromConfigs(args.Environment)
	if err != nil {
		log.Fatal("error reading task definition from file: ", err)
	}

	if args.Manifest == "" {
		log.Println("creating a new release and release file for version: ", args.ReleaseVersion)
		args.Manifest, err = createNewRelease(args.ReleaseVersion, args.CurrentBranch, tasks)
		if err != nil {
			log.Fatal("error creating new release: ", err)
		}
	}

	if err := updateTaskFromManifest(args.Manifest, tasks); err != nil {
		log.Fatal("error updating task from manifest: ", err)
	}

	deployment, err := newDeployment(tasks)
	if err != nil {
		log.Fatal("error creating new deployment: ", err)
	}

	log.Printf("registering new task for version: %s using commit: %s\n", deployment.tasks.Version, deployment.tasks.Commit)

	if err = deployment.registerTasks(); err != nil {
		log.Fatal("error registering new task: ", err)
	}

	if err = deployment.releaseAPIDocs(); err != nil {
		log.Fatal("error in release api docs to s3: ", err)
	}

	if args.NewInfra {
		log.Println("creating service for clusters with new task definitions")
		if err = deployment.createService(); err != nil {
			log.Fatal("error registering service with cluster: ", err)
		}

		return
	}

	if err := deployment.updateService(); err != nil {
		log.Fatal("error unable to update the service definitions: ", err)
	}
}

type deployment struct {
	client   *ecs.ECS
	s3Client *s3.S3
	tasks    *configs.TaskDefinitions
}

func newDeployment(tasks *configs.TaskDefinitions) (deployment, error) {
	client, err := newECSSession()
	if err != nil {
		return deployment{}, err
	}

	s3client, err := newS3Session()
	if err != nil {
		return deployment{}, err
	}

	return deployment{
		client:   client,
		s3Client: s3client,
		tasks:    tasks,
	}, nil
}

func (d deployment) registerTasks() error {
	for family, task := range d.tasks.Tasks {
		_, err := d.client.RegisterTaskDefinition(newECSTaskDefinition(family, task, d.tasks.SourceVolumes))
		if err != nil {
			return err
		}
	}

	return nil
}

// createService registers a service for a cluster from a given task definition, defaulting to the latest made
func (d deployment) createService() error {
	for family, task := range d.tasks.Tasks {
		loadBalancers := []*ecs.LoadBalancer{
			&ecs.LoadBalancer{
				ContainerName:  aws.String(family),
				ContainerPort:  task.LoadBalancerPort,
				TargetGroupArn: task.TargetGroupARN,
			},
		}

		createService := &ecs.CreateServiceInput{
			ClientToken:    aws.String(family),
			Cluster:        aws.String(task.ClusterName),
			DesiredCount:   aws.Int64(task.DesiredCount),
			ServiceName:    aws.String(family),
			TaskDefinition: aws.String(family),
			Role:           aws.String(task.RoleARN),
			LoadBalancers:  loadBalancers,
		}

		_, err := d.client.CreateService(createService)
		if err != nil {
			return err
		}
	}

	return nil
}

func (d deployment) updateService() error {
	for family, task := range d.tasks.Tasks {
		payload := &ecs.UpdateServiceInput{
			Cluster:        aws.String(task.ClusterName),
			DesiredCount:   aws.Int64(task.DesiredCount),
			Service:        aws.String(family),
			TaskDefinition: aws.String(family),
		}

		_, err := d.client.UpdateService(payload)
		if err != nil {
			return err
		}
	}

	return nil
}

// newECSTaskDefinition creates a new taskdefinition from a config file, you can have more than one container in a task def
// however we will only ever add one container per task
func newECSTaskDefinition(family string, task *configs.TaskDefinitionInfo, sourceVolumes []*configs.SourceVolume) *ecs.RegisterTaskDefinitionInput {
	volumes := make([]*ecs.Volume, len(sourceVolumes))
	for i, volume := range sourceVolumes {
		volumes[i] = &ecs.Volume{
			Name: aws.String(volume.Name),
			Host: &ecs.HostVolumeProperties{
				SourcePath: aws.String(volume.SourcePath),
			},
		}
	}

	var containers []*ecs.ContainerDefinition
	for _, container := range task.Containers {
		containers = append(containers, newContainerDefinition(container))
	}

	taskDef := &ecs.RegisterTaskDefinitionInput{
		Family:               aws.String(family),
		NetworkMode:          aws.String(task.NetworkMode),
		Volumes:              volumes,
		ContainerDefinitions: containers,
	}

	return taskDef
}

func newContainerDefinition(containerInfo *configs.ContainerInfo) *ecs.ContainerDefinition {
	portMappings := make([]*ecs.PortMapping, len(containerInfo.Ports))
	for i, mapping := range containerInfo.Ports {
		portMappings[i] = &ecs.PortMapping{
			ContainerPort: mapping.ContainerPort,
			HostPort:      mapping.HostPort,
		}
	}

	mountPoints := make([]*ecs.MountPoint, len(containerInfo.VolumeMounts))
	for i, mount := range containerInfo.VolumeMounts {
		mountPoints[i] = &ecs.MountPoint{
			ContainerPath: aws.String(mount.ContainerPath),
			ReadOnly:      aws.Bool(mount.ReadOnly),
			SourceVolume:  aws.String(mount.SourceVolume),
		}
	}

	environmentVars := make([]*ecs.KeyValuePair, len(containerInfo.EnvironmentVars))
	var i int
	for key, val := range containerInfo.EnvironmentVars {
		environmentVars[i] = &ecs.KeyValuePair{
			Name:  aws.String(key),
			Value: aws.String(val),
		}
		i++
	}

	containerDef := &ecs.ContainerDefinition{
		Cpu:          containerInfo.CPU,
		Memory:       containerInfo.Memory,
		Image:        aws.String(containerInfo.ImageName),
		Name:         aws.String(containerInfo.Name),
		Environment:  environmentVars,
		Links:        containerInfo.Links,
		PortMappings: portMappings,
		MountPoints:  mountPoints,
	}

	if containerInfo.Image != nil && len(containerInfo.Image.EntryPoint) > 0 {
		containerDef.EntryPoint = aws.StringSlice(containerInfo.Image.EntryPoint)
	}

	return containerDef
}

// createTaskFromConfigs parses the environment config file to create task definitions
func createTaskFromConfigs(envPath string) (*configs.TaskDefinitions, error) {
	eBlob, err := ioutil.ReadFile(envPath)
	if err != nil {
		return nil, err
	}

	tasks := new(configs.TaskDefinitions)
	if err = json.Unmarshal(eBlob, tasks); err != nil {
		return nil, err
	}

	return tasks, nil
}

// updateTaskFromManifest parses the manifest file and adds those values to the tasks object
func updateTaskFromManifest(manPath string, tasks *configs.TaskDefinitions) error {
	mBlob, err := ioutil.ReadFile(manPath)
	if err != nil {
		return err
	}

	manifest := new(configs.DeploymentManifest)
	if err = json.Unmarshal(mBlob, manifest); err != nil {
		return err
	}

	return tasks.UpdateFromManifest(manifest)
}

func newECSSession() (*ecs.ECS, error) {
	sess, err := session.NewSession(&aws.Config{
		Region: aws.String(endpoints.UsWest2RegionID),
	})
	if err != nil {
		return nil, err
	}
	svc := ecs.New(sess)

	return svc, nil
}

func newS3Session() (*s3.S3, error) {
	sess, err := session.NewSession(&aws.Config{
		Region: aws.String(endpoints.UsWest2RegionID),
	})
	if err != nil {
		return nil, err
	}
	client := s3.New(sess)

	return client, err
}

// createNewRelease builds a new manifest file from the defined containers that need to be made and pushed to ECR
// then writes the manifest file to disk as read only for source control and roll backs
func createNewRelease(version string, currentBranch bool, cfg *configs.TaskDefinitions) (string, error) {
	commitShaw, err := getReleaseSHA()
	if err != nil {
		// we will allow you to release a branch other than master if the flag is present
		// but this is not a good idea
		if err == ErrNotMaster && currentBranch {
			log.Println("releasing from a branch that is not master, this is ill advised")
		} else {
			return "", err
		}
	}

	if err := loginToECR(); err != nil {
		return "", err
	}

	var newImages []*configs.ManImage
	for family, task := range cfg.Tasks {
		for _, container := range task.Containers {
			var imageName string
			if container.Build != nil {
				if err = buildBinary(container.Build.BuildPath); err != nil {
					return "", err
				}

				imageName, err = pushDockerImage(version, container.Name, container.Build)
				if err != nil {
					return "", err
				}
			}

			if container.Image != nil {
				if container.Image.ImageTag != "" {
					imageName = fmt.Sprintf("%s:%s", container.Image.ImageName, container.Image.ImageTag)
				} else {
					imageName = container.Image.ImageName
				}
			}

			newImages = append(newImages, &configs.ManImage{
				Family: family,
				Name:   container.Name,
				Image:  imageName,
			})
		}

	}

	manifest := &configs.DeploymentManifest{
		CommitSHA: commitShaw,
		Version:   version,
		Images:    newImages,
	}

	return writeNewManifest(manifest)
}

func writeNewManifest(man *configs.DeploymentManifest) (string, error) {
	jBlob, err := json.Marshal(man)
	if err != nil {
		return "", err
	}

	filePath := fmt.Sprintf("releases/%s.json", man.Version)
	if err := ioutil.WriteFile(filePath, jBlob, 0444); err != nil {
		return "", err
	}

	return filePath, nil
}

// buildBinary builds the go binary for a linux 64 env to run in the container
func buildBinary(buildPath string) error {
	cmd := "go"
	cmdArgs := []string{"build", buildPath}
	generateArgs := []string{"generate", "teleopui/teleopui.go"}
	testDocGenArgs := []string{"test", "./teleopnode/..."}
	env := append(os.Environ(), "GOOS=linux", "GOARCH=amd64")

	log.Println("generating teleopui bindata from frontend")
	if err := printCmd(cmd, generateArgs, env); err != nil {
		return err
	}

	log.Println("generating API docs")
	if err := printCmd(cmd, testDocGenArgs, env); err != nil {
		return err
	}

	log.Println("building go binary")
	buildCmd := exec.Command(cmd, cmdArgs...)
	buildCmd.Env = env
	return buildCmd.Run()
}

// login to ECR with the local aws credentials in order to push our new docker images to the repo
func loginToECR() error {
	cmd := "aws"
	cmdArgs := []string{"ecr", "get-login", "--no-include-email", "--region", "us-west-2"}

	resultCmd, err := exec.Command(cmd, cmdArgs...).Output()
	if err != nil {
		return err
	}

	loginPieces := strings.Split(string(resultCmd), " ")
	if len(loginPieces) <= 1 {
		return fmt.Errorf("the pieces of the login command generated are to short size: %d", len(loginPieces))
	}
	loginPieces[len(loginPieces)-1] = strings.TrimSpace(loginPieces[len(loginPieces)-1])

	return exec.Command(loginPieces[0], loginPieces[1:]...).Run()
}

// pushDockerImage builds, tags, and pushes new docker images to the ECR repo
func pushDockerImage(version, name string, cfg *configs.BuildInfo) (string, error) {
	imageName := fmt.Sprintf("%s/%s:%s", cfg.RepoLocation, name, version)
	dockerFile := fmt.Sprintf("%s/Dockerfile", cfg.BuildPath)

	cmd := "docker"
	buildArgs := []string{"build", "-f", dockerFile, "-t", name, "."}
	tagArgs := []string{"tag", fmt.Sprintf("%s:latest", name), imageName}
	pushArgs := []string{"push", imageName}

	log.Println("building docker image")
	if err := printCmd(cmd, buildArgs, os.Environ()); err != nil {
		return "", err
	}

	if err := exec.Command(cmd, tagArgs...).Run(); err != nil {
		return "", err
	}

	log.Println("pushing docker image")
	if err := printCmd(cmd, pushArgs, os.Environ()); err != nil {
		return "", err
	}

	return imageName, nil
}

// runs a command with the args given while catching and printing STDOUT for the command
func printCmd(cmdName string, args, envs []string) error {
	cmd := exec.Command(cmdName, args...)
	cmd.Env = envs
	reader, err := cmd.StdoutPipe()
	if err != nil {
		return err
	}

	scanner := bufio.NewScanner(reader)
	go func() {
		for scanner.Scan() {
			log.Println(scanner.Text())
		}
	}()

	if err := cmd.Start(); err != nil {
		return err
	}

	return cmd.Wait()
}

// get the current branches commit SHA
func getReleaseSHA() (string, error) {
	cmd := "git"
	cmdArgs := []string{"rev-parse", "--verify", "HEAD"}
	result, err := exec.Command(cmd, cmdArgs...).Output()
	if err != nil {
		return "", err
	}

	if err := ensureMaster(); err != nil {
		return string(result), err
	}

	return string(result), nil
}

// throw error if not on the master branch
func ensureMaster() error {
	cmd := "git"
	cmdArgs := []string{"rev-parse", "--symbolic-full-name", "--abbrev-ref", "HEAD"}
	result, err := exec.Command(cmd, cmdArgs...).Output()
	if err != nil {
		return err
	}

	if string(result) != "master" {
		return ErrNotMaster
	}

	return nil
}

func (d deployment) releaseAPIDocs() error {
	for _, task := range d.tasks.Tasks {
		for _, container := range task.Containers {
			// don't do anything for tasks that don't have defined docs
			if container.Build == nil || container.Build.APIBlueprint == "" {
				continue
			}

			docsFile, err := buildDocsHTML(container.Build.APIBlueprint, container.Build.DocsBuildPath)
			if err != nil {
				return err
			}

			if err := d.uploadDocs(docsFile, container.Build.DocsS3Bucket); err != nil {
				return err
			}
		}
	}

	return nil
}

func buildDocsHTML(apibFile, docsBuildPath string) (string, error) {
	releaseFile := fmt.Sprintf("%s/output.html", docsBuildPath)

	htmlGenCmd := "aglio"
	htmlGenArgs := []string{"-i", fmt.Sprintf("%s/API.apib", docsBuildPath), "-o", releaseFile}

	if err := os.Rename(apibFile, fmt.Sprintf("%s/API.apib", docsBuildPath)); err != nil {
		return "", err
	}

	if err := printCmd(htmlGenCmd, htmlGenArgs, os.Environ()); err != nil {
		return "", err
	}

	return releaseFile, nil
}

func (d deployment) uploadDocs(uploadFile, bucketName string) error {
	file, err := os.Open(uploadFile)
	if err != nil {
		return err
	}
	defer file.Close()

	fileInfo, err := file.Stat()
	if err != nil {
		return err
	}
	size := fileInfo.Size()

	fileBuffer := make([]byte, size)
	_, err = file.Read(fileBuffer)
	if err != nil {
		return err
	}

	fileReader := bytes.NewReader(fileBuffer)
	fileType := http.DetectContentType(fileBuffer)

	payload := &s3.PutObjectInput{
		Bucket:        aws.String(bucketName),
		Key:           aws.String("index.html"),
		Body:          fileReader,
		ContentLength: aws.Int64(size),
		ContentType:   aws.String(fileType),
	}

	if _, err = d.s3Client.PutObject(payload); err != nil {
		return err
	}

	return nil
}
