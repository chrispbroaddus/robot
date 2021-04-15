# Cookbook Name:: kernel_module
# Recipe:: default
#
# Copyright 2016, Shopify Inc.

# Load all kernel modules specified in attributes
(node[:kernel_modules] || {}).each do |module_name, action|
  kernel_module module_name do
    action action
  end
end
