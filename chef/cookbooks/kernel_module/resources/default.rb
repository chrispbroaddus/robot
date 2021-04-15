#
# Cookbook Name:: kernel_module
# Resource:: default
#
# Copyright 2016, Shopify Inc.
default_action :install

property :modname, String, name_property: true, identity: true
property :load_dir, String, default: '/etc/modules-load.d'
property :unload_dir, String, default: '/etc/modprobe.d'

# Load kernel module, and ensure it loads on reboot
action :install do
  directory new_resource.load_dir do
    recursive true
  end

  file "#{new_resource.load_dir}/#{modname}.conf" do
    content modname
    notifies :run, 'execute[update-initramfs]'
  end

  execute 'update-initramfs' do
    command 'update-initramfs -u'
    action :nothing
  end

  new_resource.run_action(:load)
end

# Unload module and remove module config, so it doesn't load on reboot.
action :uninstall do
  file "#{new_resource.load_dir}/#{modname}.conf" do
    action :delete
    notifies :run, 'execute[update-initramfs]'
  end

  file "#{new_resource.unload_dir}/blacklist_#{modname}.conf" do
    action :delete
    notifies :run, 'execute[update-initramfs]'
  end

  execute 'update-initramfs' do
    command 'update-initramfs -u'
    action :nothing
  end

  new_resource.run_action(:unload)
end

# Blacklist kernel module
action :blacklist do
  file "#{new_resource.unload_dir}/blacklist_#{modname}.conf" do
    content "blacklist #{modname}"
    notifies :run, 'execute[update-initramfs]'
  end

  execute 'update-initramfs' do
    command 'update-initramfs -u'
    action :nothing
  end

  new_resource.run_action(:unload)
end

# Load kernel module
action :load do
  execute "modprobe #{new_resource.modname}" do
    not_if "lsmod | grep #{new_resource.modname}"
  end
end

# Unload kernel module
action :unload do
  execute "modprobe -r #{new_resource.modname}" do
    only_if "lsmod | grep #{new_resource.modname}"
  end
end
