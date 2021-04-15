kernel_module Cookbook
======================

This cookbook will aid in adding/removing kernel modules from a running system, and ensure they're loaded at system boot.

Requirements
------------
Requires Chef 12.5+.

### Platform
The release was tested on:

* Ubuntu 12.04
* Ubuntu 14.04

May work with or without modification on other Debian derivatives.


Recipes
-------

#### default
This recipe expects `node[:kernel_modules]` to be of the form:

```ruby
{
  raid10: :install,
  raid456: :uninstall,
  ntfs: :blacklist
}
```

and performs the actions specified on the modules listed, so you can specify modules to load/unload entirely from a role-file.

Attributes
----------

### General
* `['kernel_modules']` - Hash of modules to perform actions on using the default recipe.

Resources
---------
### `kernel_module`
This resource allows you to manage kernel modules.

#### Actions
- :install: loads the module immediately, adds an entry to `/etc/modprobe.d` to ensure it loads on boot, and updates the initramfs.
- :uninstall: unloads the module immediately, removes the configuration entry, and updates the initrams.
- :blacklist: unloads the module immediately, and adds a configuration file to blacklist the module.
- :load: loads the module immediately.
- :unload: unloads the module immediately.

#### Examples

Permanently load the `zfs` module:

```ruby
kernel_module 'zfs'
```

Unload just the `raid10` module:

```ruby
kernel_module 'raid10' do
  action :unload
end
```
