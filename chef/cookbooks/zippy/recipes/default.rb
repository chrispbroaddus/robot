#
# Cookbook:: zippy
# Recipe:: default
#
# Copyright:: 2017, The Authors, All Rights Reserved.
users_manage 'sysadmin' do
  group_id 2300
  action [:create]
  data_bag 'users'
end

include_recipe "sudo"

execute 'update_hostname' do
  command "echo #{node.name} > /etc/hostname"
  command "sed -i 's/127.0.0.1 localhost/127.0.0.1 #{node.name}.zippy #{node.name} localhost/' /etc/hosts"
  command "hostname #{node.name}"
end

route53_record "create nodename record" do
  name  "#{node.name}.zippy"
  value "#{node['ipaddress']}"
  type  "A"
  zone_id "Z332G2ACWXRXBJ"
  fail_on_error false
  action :create
end
