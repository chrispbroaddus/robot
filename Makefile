DELETED_PKGS := "packages/unity_simulator,firmware/stm,packages/unity_plugins,applications/evaulator_runner"

ci-format:
	./script/test-buildfile-formatting
	./script/test-code-formatting
	./script/test-csharp-formatting

ci-build:
	bazel --bazelrc=./bazelrc_for_ci build --python_path=/usr/bin/python3 -c opt `bazel query 'kind("cc_binary|py_binary", //...) except (attr("tags", "hardware|manual|sim_plugin", //...))' \
	--deleted_packages=packages/unity_simulator,firmware/stm,packages/unity_plugins` --deleted_packages=$(DELETED_PKGS)
	#bazel --bazelrc=./bazelrc_for_ci build //firmware/stm/... --crosstool_top=@stm32//tools/arm_compiler:toolchain --cpu=armeabi-v7a -c opt --genrule_strategy=sandboxed --spawn_strategy=sandboxed --sandbox_debug

ci-analyze-build-profile:
	bazel --bazelrc=./bazelrc_for_ci analyze-profile build.profile --html --html_details --html_histograms 

ci-code-coverage:
	bazel test --test_output=errors --config=kcov //... --deleted_packages=$(DELETED_PKGS)

ci-check-configuration:
	bazel-bin/applications/configuration_linter/configuration-linter -c config

ci-test:
	bazel --bazelrc=./bazelrc_for_ci test --python_path=/usr/bin/python3 -c opt `bazel query 'kind("_test", //...) except (attr("tags", "hardware|manual|sim_plugin", //...))'` \
	--test_output=errors --cache_test_results=no --deleted_packages=$(DELETED_PKGS)
