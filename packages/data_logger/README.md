

# How to build
## Install dependencies

```
brew install sdl2
```

## Run bazel
```
bazel build packages/data_logger:data_logger
```

# How to create an example configuration file

The data logger needs a configuration file for the sensors which contains physical connection
information and sensor parameters. You can generate an example configuration file using the following command.

```
 ./data_logger --exampleConfig=true --exampleConfigFile=/tmp/example_config.txt --logtostderr=true
```

Here is an example configuration file:

```
camera {
  streamId: "camera0"
  properties {
    data {
      key: "factory"
      value: "SimulatedCamera"
    }
    data {
      key: "height"
      value: "480"
    }
    data {
      key: "width"
      value: "640"
    }
  }
}
camera {
  streamId: "camera1"
  properties {
    data {
      key: "factory"
      value: "SimulatedCamera"
    }
    data {
      key: "height"
      value: "1080"
    }
    data {
      key: "width"
      value: "1920"
    }
  }
}
```

# How to run

Once you have a configuration file you can invoke the data logger with the following command.

```
./data_logger --configFile=/tmp/example_config.txt --dataOutputDir=/tmp/data --logtostderr=true
```
