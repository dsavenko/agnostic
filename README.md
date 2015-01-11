# Agnostic

Agnostic is a software project definition tool. The goals are:

1. Make  "Getting started" information available in one easily readable and parsable place, which is the project file;
2. Untie a project from its CI server and bring unified development process experience.

A software project from developer's point of view consists of a number of associated parts. When a new developer joins a project, he/she must somehow get this information:

* what components (modules, libraries, etc) the project consists of;
* what repositories to check out;
* where to get initial information about the project (e.g. some wiki pages);
* which tools to use;
* where are the CI servers, which jobs to watch on them;
* where are the bug trackers;
* where are the production servers, if any;
* who are the contact persons for each module/component, or where are the mailing lists;
* what are the project policies, conventions;
* etc.

Typically, all this information is passed to a new guy via e-mail or verbally. 

**The first goal** of Agnostic is to make "Getting started" information available in one easily readable and parsable place, which is the project file. Ideally, to join a project, a developer should need no more than the project file, which replaces traditional "Getting started" e-mails or wiki pages. This file describes the project with all its components, gives links to the documentation, repositories, contacts and so on. It is human-readable, in which it highly resembles traditional README files. However, it has a defined format, therefore many operations may be automated. 

Agnostic defines such project files and provides a command line tool to work with them. For instance, instead of cloning all necessary repositories manually one by one, you can just run `ag clone`.

**The second goal** of Agnostic is to untie a project from its CI server and bring unified development process experience. Don't get me wrong, CI servers are great. They're just not flexible enough, especially when compared to modern highly versitle VCSs like Git. 

Typically, project code is stored in a number of source repositories. But an important part of the information about the project (how to build it) exists as CI server jobs configuration. This information is not easily manageable. Usually, you can't easily migrate to another instance of the same CI server, not to mention migration to another CI server. Often, you can't easily run tests on local machine, because necessary scripts are not in the repository. When you need to build *several* components locally, you again need to study CI server configs to understand, in which order to build them. And so on.

Your project becomes highly coupled with the particular instance of some CI server. Sometimes, even build scripts for components exist only in the CI configs. 

All of this means, that what actually *defines* the project is its CI server. This situation seems wrong. **The second goal** of Agnostic is to change it. Agnostic project file contains all necessary information about how to build a project (build instructions, component dependencies, etc). Agnostic command line tool provides ways to utilize this information locally. E.g. you just execute `ag build` to build any component regardless of its language and build system, and `ag build up` to do upstream build (build component with all its dependencies). 

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
