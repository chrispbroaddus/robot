#
# Cookbook:: robot
# Recipe:: default
#
# Copyright:: 2017, The Authors, All Rights Reserved.

#include_recipe "openssh"
include_recipe "rsyslog"

docker_service 'default' do
  action [:create, :start]
  insecure_registry ["server1.zippy:5000"]
end

user 'zippy' do
  system true
end

group 'docker' do
  action :modify
  members 'zippy'
  append true
end

group 'dialout' do
  action :modify
  members 'zippy'
  append true
end

group 'dialout' do
  members 'zippy'
  append true
end

group 'flirimaging' do
  members 'zippy'
  gid 1001
end

apt_update 'update' do
  action :nothing
end

execute 'add_verse' do
  command "apt-add-repository universe"
  command "apt-add-repository multiverse"
  notifies :update, 'apt_update[update]', :immediately
end

execute 'add_nvidia' do
  command "sudo apt-get remove --purge nvidia-*"
  command "sudo add-apt-repository ppa:graphics-drivers"
  notifies :update, 'apt_update[update]', :immediately
end

apt_package 'gpsd' do
  action :install
  options '-y'
end

execute 'update_grub' do
  command "update-grub"
  action :nothing
end

template "/etc/default/grub" do
  source "grub.erb"
  owner "root"
  group "root"
  mode "0755"
  notifies :run, 'execute[update_grub]', :immediately
end

template "/lib/udev/gpsd.hotplug" do
  source "gpsd.hotplug.erb"
  owner "root"
  group "root"
  mode "0755"
end

template "/lib/udev/rules.d/25-gpsd.rules" do
  source "25-gpsd.rules.erb"
  owner "root"
  group "root"
  mode "0755"
end

template "/etc/default/gpsd" do
  source "gpsd.erb"
  owner "root"
  group "root"
  mode "0755"
end

template "/etc/udev/rules.d/40-flir.rules" do
  source "40-flir.rules.erb"
  owner "root"
  group "root"
  mode "0755"
end

apt_package 'chrony' do
  action :install
  options '-y'
end

template "/etc/chrony.conf" do
  source "chrony.conf.erb"
  owner "root"
  group "root"
  mode "0755"
end

apt_package 'ptpd' do
  action :install
  options '-y'
end

template "/etc/default/ptpd" do
  source "ptpd.erb"
  owner "root"
  group "root"
  mode "0755"
end

apt_package 'nvidia-modprobe' do
  action :install
  options '-y'
end

apt_package 'nvidia-375' do
  action :install
  options '-y'
end

bash "install_nvidia_docker" do
  user "root"
  cwd "/tmp"
  code <<-EOS
    wget https://github.com/NVIDIA/nvidia-docker/releases/download/v1.0.1/nvidia-docker_1.0.1-1_amd64.deb
    dpkg -i nvidia-docker*.deb && rm nvidia-docker*.deb
  EOS
end

#bash "install_cudnn" do
#  user "root"
#  cwd "/tmp"
#  code <<-EOS
#    wget https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/cudnn-8.0-linux-x64-v5.1.tgz
#    tar xzvf cudnn-*
#    rm cudnn*.tgz
#    cp -P cuda/include/cudnn.h /usr/include
#    cp -P cuda/lib64/libcudnn* /usr/lib/x86_64-linux-gnu/
#    chmod a+r /usr/lib/x86_64-linux-gnu/libcudnn*
#    rm -rf /tmp/cuda/
#  EOS
#end

network_interface node['wireless']['interface'] do
  custom "wpa-ssid" => node['wireless']['ssid'], "wpa-psk" => node['wireless']['password']
  reload false
#  metric 10
end

#network_interface node['lte']['interface'] do
#  reload false
#  metric 100
#end

network_interface node['vcu']['interface'] do
  bootproto 'static'
  address node['vcu']['address']
  netmask '255.255.255.0'
  reload false
end

reboot 'app_requires_reboot' do
  action :request_reboot
  reason 'Need to reboot when the run completes successfully.'
  delay_mins 1
end
