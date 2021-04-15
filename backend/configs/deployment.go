package configs

import (
	"fmt"
)

// TaskDefinitions map of task family to definition
type TaskDefinitions struct {
	// Tasks are a map of task definition "family" name to config information needed for aws
	Tasks map[string]*TaskDefinitionInfo `json:"tasks"`
	// SourceVolumes are all of the volumes that we need to exist on the host for all task definitions
	SourceVolumes []*SourceVolume `json:"source_volumes"`
	// version specific information
	Version string `json:"-"`
	Commit  string `json:"-"`
}

// TaskDefinitionInfo the collective information needed to define a cluster
type TaskDefinitionInfo struct {
	// DesiredCount is how many containers we want to be running
	DesiredCount int64 `json:"desired_count"`
	// identifying information of the cluster we want this task to run on
	ClusterName string `json:"cluster_name"`
	ClusterARN  string `json:"cluster_arn"`
	// name of the loadbalancer we want to be in front of the containers, this is a pointer so if it's not defined we can handle that
	LoadBalancerName *string `json:"lb_name"`
	// port used for the load balancer
	LoadBalancerPort *int64 `json:"lb_port"`
	// the target group, collection of port mappings we want an alb to wire up with the contairs, also a pointer so we can handle nil better
	TargetGroupARN *string `json:"target_group_arn"`
	// IAM role that is in charge of the ecs service that runs the task, so it can pull images and make other role defined aws operations
	RoleARN string `json:"role_arn"`
	// mode that the containers are run, values are: node, bridge, and host default is bridge
	NetworkMode string `json:"network_mode"`
	// Containers are the containers used for this task
	Containers []*ContainerInfo `json:"containers"`
}

// ContainerInfo contains all of the data needed to define containers in a task
type ContainerInfo struct {
	// CPU and Memory set hard limits for the containers that run in this task
	CPU    *int64 `json:"cpu_usage,omitempty"`
	Memory *int64 `json:"memory_usage,omitempty"`
	// ImageName is pulled from the manifest and is the image we want to run in the task
	ImageName string `json:"-"`
	// task definition name
	Name string `json:"name"`
	// define linked containers
	Links []*string `json:"links"`
	// linking the source volumes on the host to defined paths on the container
	VolumeMounts []*VolumeMount `json:"volumes"`
	// port mappings that the container needs
	Ports []*PortMapping `json:"port_maps"`
	// environment vars for containers
	EnvironmentVars map[string]string `json:"environment"`
	// information need to build the image itself
	Build *BuildInfo `json:"image_build"`
	Image *ImageInfo `json:"image_info"`
}

// SourceVolume defines the path and name of a volume mounted folder
type SourceVolume struct {
	// path to the volume mounted folder on the host
	SourcePath string `json:"source_path"`
	// name that is referenced by the task definition itself
	Name string `json:"name"`
}

// VolumeMount information for the volume mounts we want
type VolumeMount struct {
	// path to place the volume mounted folder
	ContainerPath string `json:"container_path"`
	// setting the contants to read only
	ReadOnly bool `json:"read_only"`
	// name of the source volume that we want to mount
	SourceVolume string `json:"source_name"`
}

// PortMapping maps a container to a host, if you want to use an application load balancer set Container but not host
// application load balancers or ALB load balance for a dynamic range of host ports exposed
// so we can have multiple containers on the same host exposing the same port
type PortMapping struct {
	// port exposed from the container
	ContainerPort *int64 `json:"container_port"`
	// port that we want the host to expose for the given container port
	HostPort *int64 `json:"host_port"`
}

// BuildInfo contains the information necessary to build the docker image and upload it
type BuildInfo struct {
	// path to the build location and Dockerfile
	BuildPath string `json:"build_path"`
	// docker image repo path
	RepoLocation string `json:"docker_repo"`
	// path to the api packages
	APIBlueprint string `json:"apib_file"`
	// path to the build location for the api docs
	DocsBuildPath string `json:"docs_path"`
	// s3 bucket to host to the docs
	DocsS3Bucket string `json:"docs_bucket"`
}

// ImageInfo is used instead of BuildInfo to define image and entrypoint
type ImageInfo struct {
	// ImageName path to the prebuilt image
	ImageName string `json:"image"`
	//ImageTag tag for the prebuilt image we want to deploy
	ImageTag string `json:"image_tag"`
	// EntryPoint is used to define the entrypoint for the image
	EntryPoint []string `json:"entry_point"`
}

// UpdateFromManifest adds the new images to the corresponding task definitions from the config file
func (t *TaskDefinitions) UpdateFromManifest(man *DeploymentManifest) error {
	for _, imageInfo := range man.Images {
		task, ok := t.Tasks[imageInfo.Family]
		if !ok {
			return fmt.Errorf("unable to assign the task with family: %s an image since it's not present in the config", imageInfo.Family)
		}

		for _, container := range task.Containers {
			if container.Name == imageInfo.Name {
				container.ImageName = imageInfo.Image
			}
		}
	}

	t.Version = man.Version
	t.Commit = man.CommitSHA

	return nil
}

// DeploymentManifest is the version controlled file for all of the containers built for a release
type DeploymentManifest struct {
	// version of the deploy
	Version string `json:"version"`
	// commit SHA that was deployed
	CommitSHA string `json:"commit"`
	// image information for all of the images deployed
	Images []*ManImage `json:"images"`
}

// ManImage contains the task definition family this image is for
type ManImage struct {
	// task definition family that was deployed
	Family string `json:"task_family"`
	// container name
	Name string `json:"container_name"`
	// fully clarified docker image name and tag
	Image string `json:"image"`
}
