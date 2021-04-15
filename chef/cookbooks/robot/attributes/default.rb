#default['openssh']['server']['password_authentication']= "no"
default['rsyslog']['modules'] = ['imuxsock', 'imklog', 'imudp']
default['rsyslog']['protocol'] = 'udp'
default['rsyslog']['input'] = 'input(type="imudp" port="514")'
default['vcu']['interface'] = 'enp2s0f0'
default['vcu']['address'] = '192.168.100.100'
default['wireless']['interface'] = 'wlp5s0'
default['wireless']['ssid'] = 'Zippy5G'
default['wireless']['password'] = ''
default['wireless']['gateway'] = '172.16.36.1'
default['lte']['interface'] = 'enp2s0f1'
