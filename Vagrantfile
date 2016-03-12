# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
    config.vm.provider "virtualbox" do |vb|
        vb.memory = "2404"
        vb.cpus = "4"
    end

    config.vm.define "linux64" do |linux64|
        linux64.vm.box = "linux64"
        linux64.vm.box = "bugyt/archlinux"

        linux64.vm.provision "shell", inline: <<-SHELL
            sudo pacman -Syu --noconfirm
            sudo pacman --noconfirm -S mono python-pip
            sudo pip install -r requirements.txt
        SHELL
    end
end
