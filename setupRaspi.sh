# Some necessary SSH configuration if connection is not possible
# nvim /etc/ssh/sshd_config
# uncomment UseDNS no
# Add to end of file IPQoS cs0 cs0
# service ssh restart


git clone https://github.com/boffmann/Dotfiles.git
mkdir ~/.config
cp -r ~/repos/Dotfiles/nvim ~/.config/
sudo apt update
sudo apt install snapd
sudo snap install snapd
sudo apt install nvim --classic
sudo ln -s /snap/nvim/current/usr/bin/nvim /bin/nvim
git clone https://github.com/boffmann/jellED
sudo raspi-config

