# Chomp Turret Plugin

## Configuration

Need to update node.  Easiest is to use NVM

```
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.1/install.sh | bash
nvm install node
```

To build the widgets, need npm and vue-cli

```
sudo apt install npm
npm install
```

Now build the widgets

```
npm run build
```

## Building the plugin

1. <Path to COSMOS installation>\cosmos-control.sh cosmos rake build VERSION=X.Y.Z
   - VERSION is required
   - gem file will be built locally

## Upload plugin

1. Go to localhost:2900/tools/admin
1. Click the paperclip icon and choose your plugin.gem file
1. Click Upload
