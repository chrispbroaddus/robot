#!/bin/bash
set -e
export HOME=/root
export USER=root
export DISPLAY=:1.0

(bash /startup.sh) &
sleep 60

set +e
/opt/Unity/Editor/Unity -batchmode -nographics -runEditorTests -projectPath /zippy/packages/unity_simulator/ZippySimUnity/
return_value=$?
set -e
cat packages/unity_simulator/ZippySimUnity/TestResults*
exit $return_value
