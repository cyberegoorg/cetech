# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
    config.vm.provider "virtualbox" do |vb|
        vb.memory = "2404"
        vb.cpus = "4"
    end

    config.vm.define "linux64" do |linux64|
        linux64.vm.box = "ubuntu/trusty64"

        linux64.vm.synced_folder ".", "/home/vagrant/cetech", mount_options: ['dmode=774','fmode=775']

        linux64.vm.provision "shell", inline: <<-SHELL
            sudo apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 3FA7E0328081BFF6A14DA29AA6A19B38D3D831EF
            echo "deb http://download.mono-project.com/repo/debian wheezy main" | sudo tee /etc/apt/sources.list.d/mono-xamarin.list
            sudo apt-get update

            sudo apt-get install git python3-pip mono-complete mono-vbnc autoconf libtool libgl1-mesa-dev
            sudo mozroots --import --sync
        SHELL
    end
end
