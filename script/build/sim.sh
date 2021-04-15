#!/bin/bash

# Guarantee that if any statement fails, the script bails immediate. 
set -e

export HOME=/root
export USER=root
export DISPLAY=:1.0

(bash /startup.sh) &
sleep 60

set +e
/opt/Unity/Editor/Unity -batchmode -nographics -buildLinux64Player /zippy/sim -projectPath /zippy/packages/unity_simulator/ZippySimUnity/ -quit
return_value=$?
set -e

# Slack ops-notifications channel with an @mention for dan
if [ "$return_value" -ne "0" ]; then
  curl -X POST -H 'Content-type: application/json' --data '{"text":"<@U68HGRH44> sim did not build"}' https://hooks.slack.com/services/$SLACK_HOOK
fi


mkdir ZippySimulator
mv sim ZippySimulator/sim.x86_64
mv sim_Data ZippySimulator
mv simulator_config ZippySimulator
mv packages/unity_simulator/README.md ZippySimulator
echo $COMMIT > ZippySimulator/.build
tar czvf ZippySimulator.tar.gz ZippySimulator
aws s3 cp --region us-west-2 ZippySimulator.tar.gz s3://zippysimulator/releases/$BRANCH/ZippySimulator.tar.gz
