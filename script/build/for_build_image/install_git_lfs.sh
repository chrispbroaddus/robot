#!/bin/bash

# Guarantee that if any command returns non-zero, the script returns immediately
# with a non-zero exit.
set -e

VERSION="2.1.1"
ARCHIVE="git-lfs-linux-amd64-${VERSION}.tar.gz"
DEST="git-lfs-linux-amd64-${VERSION}"
URL="https://github.com/git-lfs/git-lfs/releases/download/v${VERSION}/git-lfs-linux-amd64-${VERSION}.tar.gz"

# Install git lfs
mkdir "$DEST"
curl -L "$URL" | tar xvz --strip-components=1 -C "$DEST"
$DEST/install.sh
git lfs version

# add github SSH key to list of known hosts
# to update this line:
#   $ ssh-keyscan -t github.com
mkdir -p ~/.ssh
echo "github.com ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAQEAq2A7hRGmdnm9tUDbO9IDSwBK6TbQa+PXYPCPy6rbTrTtw7PHkccKrpp0yVhp5HdEIcKr6pLlVDBfOLX9QUsyCOV0wzfjIJNlGEYsdlLJizHhbn2mUjvSAHQqZETYP81eFzLQNnPHt4EVVUh7VfDESU84KezmD5QlWpXLmvU31/yMf+Se8xhHTvKSCZIFImWwoG6mbUoWf9nzpIoaSjB+weqqUUmpaaasXVal72J+UX2B+2RPW3RcT0eOzQgqlJL3RKrTJvdsjE3JEAvGq3lGHSZXy28G3skua2SmVi/w4yCE6gbODqnTWlg7+wC604ydGXA8VJiS5ap43JXiUFFAaQ==" > ~/.ssh/known_hosts

# add SSH key for git access
echo -e $GIT_LFS_SSH_KEY > $HOME/.ssh/id_rsa
chmod 600 $HOME/.ssh/id_rsa
