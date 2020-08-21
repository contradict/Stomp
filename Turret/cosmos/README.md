## How I got this to work

* Install some build dependencies

    $ sudo apt-get install -y build-essential libssl-dev libreadline-dev zlib1g-dev libqt4-dev cmake

* Install [rbenv](https://github.com/rbenv/rbenv#basic-github-checkout)

    $ git clone https://github.com/rbenv/rbenv.git ~/.rbenv
    $ cd ~/.rbenv && src/configure && make -C src && cd -

* Add `~/.rbenv/bin` to your `$PATH` for access to the `rbenv` command-line utility.

    $ echo 'export PATH="$HOME/.rbenv/bin:$PATH"' >> ~/.bash_profile && . ~/.bash_profile

* Run `~/.rbenv/bin/rbenv init` and follow the instructions to set up rbenv integration with your shell.

    $ eval "$(rbenv init -)"

* Install [ruby-build](https://github.com/rbenv/ruby-build#installation), which provides the rbenv install command that simplifies the process of installing new Ruby versions.

    $ mkdir -p "$(rbenv root)"/plugins
    $ git clone https://github.com/rbenv/ruby-build.git "$(rbenv root)"/plugins/ruby-build

* build ruby 2.4.4

    $ CONFIGURE_OPTS="--enable-shared" rbenv install 2.4.2

* activate ruby 2.4.2

    $ rbenv local 2.4.2

* install bundler

    $ gem install bundler

* Install this package

    $ bundle install

* run `ruby ./Launcher` in this directory to start COSMOS
