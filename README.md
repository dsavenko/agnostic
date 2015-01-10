# Agnostic

Project definition tool. 

## Samples

There are some samples in the `samples` directory. Go to a subdirectory and run `ag clone`.

1. `sample/agnostic` - dogfood project of Agnostic itself. 

## Build

Download and build [libyaml](http://pyyaml.org/wiki/LibYAML) (no need to install).

    hg clone https://bitbucket.org/xi/libyaml yaml
    cd yaml
    ./bootstrap
    ./configure
    make
    sudo make install 

Make agnostic

    make 
