# COSMOS

Ball Aerospace [COSMOS](https://cosmosrb.com/) is the tool we use for telemetry
dashboards. The version we use is modified with some additional plotting
capabilities and can be downloaded [here](https://github.com/contradict/COSMOS).

## Installation

* Install some build dependencies
```bash
$ sudo apt-get install -y build-essential libssl-dev libreadline-dev zlib1g-dev libqt4-dev cmake libpq-dev
```
* Install [rbenv](https://github.com/rbenv/rbenv#basic-github-checkout)

```bash
$ git clone https://github.com/rbenv/rbenv.git ~/.rbenv
$ cd ~/.rbenv && src/configure && make -C src && cd -
```

* Add `rbenv` to your `~/.bashrc` to ensure it is loaded every login. Add these
    two lines somewhere in `~/.bashrc`

```bash
export PATH="~/.rbenv/bin":$PATH
eval "$(rbenv init -)"
```

* Add rbenv to your current path and run `~/.rbenv/bin/rbenv init` to enable
    rbenv in your current environment. Alternatively, open a new shell and the
    lines added to `~/.bashrc` should do the same thing.

```bash
$ export PATH=~/.rbenv/bin:${PATH}
$ eval "$(rbenv init -)"
```

* Install [ruby-build](https://github.com/rbenv/ruby-build#installation), which provides the rbenv install command that simplifies the process of installing new Ruby versions.

```bash
$ mkdir -p "$(rbenv root)"/plugins
$ git clone https://github.com/rbenv/ruby-build.git "$(rbenv root)"/plugins/ruby-build
```

* build ruby 2.5.8

```bash
$ rbenv install 2.5.8
```

* Ensure you are currently in the `Stomp/COSMOS` directory and activate ruby 2.5.8

```bash
Stomp/COSMOS $ rbenv local 2.5.8
```

* install bundler (still in `Chomp/COSMOS` directory)

```bash
Stomp/COSMOS $ gem install bundler
```

* Install this package (still in `Chomp/COSMOS` directory)

```bash
Stomp/COSMOS $ bundle install
```

* run `ruby ./Launcher` in this directory to start COSMOS

```bash
Stomp/COSMOS $ ruby ./Launcher
```

## Development on the custom fork of COSMOS

There is much I still don't understand about how to do this correctly. The above
instructions for installing `ruby 2.5.8` are a good start.

* Our fork is stored [here](https://github.com/contradict/COSMOS)

* After cloning, I did

```bash
~/src/COSMOS $ rbenv local 2.5.8      # Configure this directory with ruby 2.5.8
~/src/COSMOS $ bundle install         # Install development dependencies
~/src/COSMOS $ bundle exec rake build # Build the compiled parts
```
* Run the application with `bundle exec ruby demo/Launcher` to run the app in
  the demo project.

* run `bundle exec rake` to execute the tests
