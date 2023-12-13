#!/usr/bin/env python

from mininet.net import Mininet
from mininet.node import Controller, RemoteController, OVSController
from mininet.node import CPULimitedHost, Host, Node
from mininet.node import OVSKernelSwitch, UserSwitch
from mininet.node import IVSSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import TCLink, Intf
from subprocess import call

def myNetwork():

    net = Mininet( topo=None,
                   build=False,
                   ipBase='10.0.0.0/8')

    info( '*** Adding controller\n' )
    c0=net.addController(name='c0',
                      controller=Controller,
                      protocol='tcp',
                      port=6633)

    c1=net.addController(name='c1',
                      controller=Controller,
                      protocol='tcp',
                      port=6635)

    c2=net.addController(name='c2',
                      controller=Controller,
                      protocol='tcp',
                      port=6634)

    info( '*** Add switches\n')
    s1 = net.addSwitch('s1', cls=OVSKernelSwitch)
    s2 = net.addSwitch('s2', cls=OVSKernelSwitch)
    s3 = net.addSwitch('s3', cls=OVSKernelSwitch)
    s4 = net.addSwitch('s4', cls=OVSKernelSwitch)
    s5 = net.addSwitch('s5', cls=OVSKernelSwitch)
    s6 = net.addSwitch('s6', cls=OVSKernelSwitch)
    s7 = net.addSwitch('s7', cls=OVSKernelSwitch)

    info( '*** Add hosts\n')
    h1 = net.addHost('h1', cls=Host, ip='10.0.0.1', mac = '36:16:d6:40:1a:34', defaultRoute=None) 
    h2 = net.addHost('h2', cls=Host, ip='10.0.0.2', mac = 'b2:4b:96:10:9d:42', defaultRoute=None)
    h3 = net.addHost('h3', cls=Host, ip='10.0.0.3', mac = 'ce:1e:82:45:b5:a6', defaultRoute=None)
    h4 = net.addHost('h4', cls=Host, ip='10.0.0.4', mac = 'f6:ba:3a:3e:94:8a', defaultRoute=None)
    h5 = net.addHost('h5', cls=Host, ip='10.0.0.5', mac = '7e:3d:80:0c:e2:2a', defaultRoute=None)
    h6 = net.addHost('h6', cls=Host, ip='10.0.0.6', mac = '8a:fb:2b:2e:41:1c', defaultRoute=None)
    h7 = net.addHost('h7', cls=Host, ip='10.0.0.7', mac = '26:15:a3:cd:fd:8a', defaultRoute=None)

    info( '*** Add links\n')
    net.addLink(s1, h1)
    net.addLink(s1, h2)
    net.addLink(s2, h3)
    net.addLink(s1, s5)
    net.addLink(s5, s2)
    net.addLink(s3, h4)
    net.addLink(s3, h5)
    net.addLink(s4, h7)
    net.addLink(s3, h6)
    net.addLink(s4, s6)
    net.addLink(s6, s3)
    net.addLink(s6, s7)
    net.addLink(s7, s5)

    info( '*** Starting network\n')
    net.build()
    info( '*** Starting controllers\n')
    for controller in net.controllers:
        controller.start()

    info( '*** Starting switches\n')
    net.get('s1').start([c0])
    net.get('s2').start([c0])
    net.get('s3').start([c1])
    net.get('s4').start([c1])
    net.get('s5').start([c2])
    net.get('s6').start([c2])
    net.get('s7').start([c2])

    info( '*** Post configure switches and hosts\n')

    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    myNetwork()

