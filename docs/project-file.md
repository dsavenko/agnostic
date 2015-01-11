
# Project file

*WARNING! Project file format, described here, may and, most likely, will be changed in the future. I will try to keep this file up-to-date.*

Agnostic project file describes the project with all its components, gives links to the documentation, repositories, contacts and so on. Ideally, to join a project, a developer should need no more than the project file, which replaces traditional "Getting started" e-mails or wiki pages.

The project file format is not stricly specified, so it may vary depending on the needs of a particular project. Common and recognizeable by the command line tool parts are described below. Any project should respect these conventions. 

## Top-level structure

A project file is a [YAML](http://www.yaml.org/spec/1.2/spec.html) file. It contains at least one YAML document (for the project itself), and, optionally, more documents for components. Documents are separated by a line of three dashes (`---`).

## Project document

The project document must present in the file once. It should be the first document in the file. It must have top-level mapping node `project`.

The following key/value pairs are required for the `project` node:

* `name`: the project's full name.

The following key/value pairs are optional for the `project` node:

* `description`: human-readable description of the project;
* `bugs`: URL of the bug tracker;
* `docs`: a *list* of documentation sources. Each item is either a URL, or a human-readable text;
* `tools`: a *list* of tools to use. Each item is a mapping node, described below.

The following key/value pairs are required for each tool node:

* `name`: the tool's name.

The following key/value pairs are optional for each tool node:

* `description`: human-readable description of the tool;
* `info`: URL to general resources about the tool, like the tool's site;
* `download`: direct link to download the tool.

## Component document

A component document describes a single component of the project. If the project is not split into components (e.g. it resides in a single repository and doesn't depend on anything unusual), there *should* still be one component to describe the project's sole source location.

A component document must have top-level mapping node `component`. 

The following key/value pairs are required for the `component` node:

* `name`: the component's full name.

The following key/value pairs are optional for the `component` node:

* `description`: human-readable description of the component;
* `alias`: the component's short name;
* `git` or `hg` (but not both): the component's source repository (Git or Mercurial) in a form suitable to run `git clone` or `hg clone`;
* `build`: shell script to build the component locally (developer build);
* `integrate`: shell script to perform integartion build;
* `buildAfter`: a list of names or aliases of other components from this file, which should be built before this component.

## Project file samples

Project file samples are in the [samples](../samples/) directory of this repository. You may `cd` to a subdirectory of the [samples](../samples/) directory and run `ag clone` to see, how it works. 

1. [samples/agnostic](../samples/agnostic) - dogfood project of Agnostic itself.
