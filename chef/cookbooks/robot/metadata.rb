name 'robot'
maintainer 'Dan Zezula'
maintainer_email 'dan@zippy.ai'
license 'All Rights Reserved'
description 'Installs/Configures robot'
long_description 'Installs/Configures robot'
version '0.1.0'
chef_version '>= 12.1' if respond_to?(:chef_version)

# The `issues_url` points to the location where issues for this cookbook are
# tracked.  A `View Issues` link will be displayed on this cookbook's page when
# uploaded to a Supermarket.
#
# issues_url 'https://github.com/<insert_org_here>/robot/issues'

# The `source_url` points to the development repository for this cookbook.  A
# `View Source` link will be displayed on this cookbook's page when uploaded to
# a Supermarket.
#
# source_url 'https://github.com/<insert_org_here>/robot'
depends 'docker', '~> 2.0'
depends 'openssh', '~> 2.0'
depends 'rsyslog', '~> 6.0'
depends 'network_interfaces_v2'
