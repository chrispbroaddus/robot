#!/usr/bin/python
import requests
import os
import json
from time import sleep



while True:
    r = requests.get('http://localhost/pull')
    if r.text != 'OK':
        data = json.loads(r.json())
        os.system('sudo docker pull server1.zippy:5000/zippy-build-$(echo {0} | tr "[:upper:]" "[:lower:]"):{0}'.format(data['commit']))
        cmd = 'sudo nvidia-docker run -it --rm server1.zippy:5000/zippy-build-$(echo {0} | tr "[:upper:]" "[:lower:]"):{1} bash -c "mkdir /root/.aws && printf \\"[default]\\naws_access_key_id = {2}\\naws_secret_access_key = {3}\\" > /root/.aws/credentials && bazel --bazelrc=./bazelrc_for_ci build --action_env=BRANCH={0} --action_env=COMMIT={1} //applications/object_detection_evaluator:run_evaluator"'.format(data['branch'], data['commit'], data['AWS_ACCESS_KEY_ID'], data['AWS_SECRET_ACCESS_KEY'])
        os.system(cmd)
        os.system('sudo docker rmi -f server1.zippy:5000/zippy-build-$(echo {0} | tr "[:upper:]" "[:lower:]"):{0}'.format(data['commit']))
    else: print 'ok'
    sleep(300)
