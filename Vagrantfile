# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
    config.vm.provider "virtualbox" do |vb|
        vb.memory = "2404"
        vb.cpus = "4"

        vb.customize ["modifyvm", :id, "--usb", "off"]
        vb.customize ["modifyvm", :id, "--usbehci", "off"]
    end

    config.vm.define "linux64" do |linux64|
        linux64.vm.box = "bugyt/archlinux"

        linux64.vm.provision "shell", inline: <<-SHELL
            sudo pacman -Syu --noconfirm
            sudo pacman --noconfirm -S mono python-pip
            sudo pip install -r requirements.txt
        SHELL
    end

    config.vm.define "osx" do |osx|
        osx.vm.box = "jhcook/osx-elcapitan-10.11"
        osx.vm.synced_folder ".", "/Users/vagrant/cetech"

        osx.vm.provision "shell", inline: <<-SHELL
            /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
        SHELL
    end
end
