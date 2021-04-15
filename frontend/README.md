# Zippy Teleop Interface 🤖

## Running TeleopUI
For most cases, you can get by with starting the Teleop server by running:
```bash
./backend/scripts/run.sh [-b BRANCH_NAME]
```
This will pull the latest container from our private Docker registry and run the `teleop-server` with a port exposed on `http://localhost:8000`. If the branch (`-b`) option is omitted, the current branch will be used.

### Futher debugging
You can debug the Teleop server further by fetching and running the container manually, as such:
```bash
# Replace BRANCH_NAME with the branch you want to see
docker pull server1.zippy:5000/teleop-build:BRANCH_NAME
docker run -it --entrypoint /bin/bash -p 8000:8000 server1.zippy:5000/teleop-build:BRANCH_NAME
```

## Developer setup
Before you begin, ensure that you have [NodeJS](https://nodejs.org/en/) version 7 or above installed. On *nix systems, it's recommended that you install Node using [Node Version Manager (nvm)](https://github.com/creationix/nvm) to better manage multiple versions of Node on your machine. Our project prefers **Node 7.0.0 or above**.

### 1. Install NVM (Node Version Manager)
```
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.4/install.sh | bash
nvm install 7.7.4
nvm use 7.7.4
# Verify node is installed
node -v
```

### 2. Install `yarn` package manager
Yarn is a dependency management tool that's built on top of Node's NPM and is  faster overall. Our project uses Yarn for consistent dependency resolutions across machines.

Install it globally:
```
npm install -g yarn
```


### 3. Install dependencies
```
# zippy/frontend/
yarn install
```

### 4. Build the project

#### Hot Reloading-enabled development build
This process will read environment variables off of `.env.development` or `.env.development.local` if it exists (see [Changing environments locally](#changing-environments-locally)) and will start a hot-reloading server at `http://localhost:3000`.
```
yarn start
# Frontend should now be accessible @ http://localhost:3000
```

#### Production build
This process will load environment variables off of `.env.production` or `.env.production.local` if it exists (see [Changing environments locally](#changing-environments-locally)).
You will first need to install a simple web server for this to work. We suggest using `serve` via `npm install -g serve`.
```
yarn build
serve -s build -p 3000
# Frontend should be accessible at http://localhost:3000
```

## Build & Deployment Process
At a high level, the backend and the frontend assets are built into a single binary file called `bindata.go` and served alongside the backend when running `teleopui`. To manaully generate this binary, run:
`go generate backend/teleopui/teleopui.go`

## Changing environments locally

By default, any **production builds** will point to `http://mission-control.zippy.ai` and **development builds** will point to `http://localhost:8000`. To change the default behavior when running on your local machine, copy the `.env` file for the environment you want to target and append `.local` to the filename, as such:

#### Development build
```
cp .env.production .env.production.local
```
#### Production build
```
cp .env.production .env.production.local
```

Because all `.env.*.local` files are ignored by Git, this will allow you to build for a specific environment using a local copy of environment variables without needing to modify tracked files.

| Build | `.env` file | Default API Address |
| --- | --- | --- |
| Development | `.env.development`, `.env.development.local`  | http://localhost:8000 |
| Production | `.env.production`, `.env.production.local` | http://mission-control.zippy.ai |

## Tech stack
- **Framework**: [React](https://facebook.github.io/react/), project seeded with [Create React App](https://github.com/facebookincubator/create-react-app)
- **State Management**: [Redux](http://redux.js.org/), [Thunk](https://github.com/gaearon/redux-thunk)
- **HTTP**: [Axios](https://github.com/mzabriskie/axios)
- **CSS Preprocessor**: [SASS](http://sass-lang.com/) without CSS modules
- **Bundler**: [Webpack 2](https://webpack.github.io/)
