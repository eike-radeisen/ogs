+++
date = "2018-02-27T11:00:13+01:00"
title = "Introduction"
author = "Lars Bilke"
weight = 1

aliases = [ "/docs/userguide/",
            "/docs/quickstart/",
            "/docs/quickstart/basics/quickstart" ] # Redirect for Hydrology III tutorial

[menu.docs]
name = "User Guide"
identifier = "userguide"
weight = 1
post = "Download, install and run an OGS benchmark in 5 minutes! No development setup required."
[menu.docs.params]
category = "Beginner"
+++

## Installation

There are various ways to obtain a (pre-built) running version of OpenGeoSys (OGS) on your machine. You can have OGS for different
operating systems including Windows, Linux, and macOS. The basic modelling platform is available for all operating systems.
The different operating systems and installation methods give you the feature matrix:

| Operating system / Installation method                                                                                       | [Processes](/docs/userguide/blocks/processes/)                                    | [MFront](/docs/userguide/features/mfront/) | [PETSc]({{< ref "parallel_computing_mpi" >}})
| ---------------------------------------------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------------: | :----------------------------------------: | :-------------------------------------------:
| <i class="fab fa-windows"></i> Windows / [pip](#install-via-pip) & [binary download](#alternative-install-via-binary-downloads) | `TH2M` disabled, [see issue](https://gitlab.opengeosys.org/ogs/ogs/-/issues/3197) |                     ❌                      |                       ❌
| <i class="fab fa-apple"></i> macOS / [pip](#install-via-pip)                                                                    |                                        All                                        |                     ✅                      |                       ❌
| <i class="fab fa-linux"></i> Linux / [pip](#install-via-pip) & [Serial Container]({{< ref "container.md" >}})                   |                                        All                                        |                     ✅                      |                       ❌
| <i class="fab fa-linux"></i> Linux / [PETSc Container]({{< ref "container.md" >}})                                              |                                        All                                        |                     ✅                      |                       ✅

<div class="note">

### Using Linux binaries on other operating systems

Please note that you can use Linux binaries installed via `pip` or in the form of a container also on other operating systems.

For Windows we recommend the [Windows Subsystem for Linux (WSL)]({{< ref "wsl" >}}).

On macOS you can use either a virtual machine (e.g. via [UTM](https://docs.getutm.app/installation/macos/)) or run a [Docker container]({{< ref "container.md#with-docker" >}}).

</div>

## Install via pip

A straightforward way of installing OGS is via Python's [`pip`-tool](https://packaging.python.org/en/latest/tutorials/installing-packages/):

```bash
# Optional: create a Python virtual environment, see below
python -m venv .venv      # or `python3 -m venv .venv`
source .venv/bin/activate # Linux / macOS; for Windows: .\venv\Scripts\activate

# Install ogs' pip package
pip install ogs
```

Please note that this requires a Python installation (version 3.8 - 3.11) with the `pip`-tool.

We recommend using Python within a [virtual environment](https://docs.python.org/3/library/venv.html) to keep possible
conflicts of different Python-packages localised. If you use `pip` for installation of OGS in a virtual environment and you
activate the virtual environment, then OGS and its tools are automatically also in the `PATH`. If the virtual environment is
not activated you may still use OGS, but either have to give the full path to `ogs` being located in the `bin` folder (or `Scripts` folder on Windows) of the
virtual environment, or add this path to your `PATH`-environment. Moreover, `pip` may print instructions which directory needs
 to be added to the `PATH`.

You could also use [`pipx`](https://pypa.github.io/pipx/) for installation into an isolated environment.

<div class="note">

### Get current development version with `pip`

The following command will download the latest development version:

```bash
pip install --pre --index-url https://gitlab.opengeosys.org/api/v4/projects/120/packages/pypi/simple ogs
```

</div>

<div class='win'>

<div class="note">

### Alternative: Install via binary downloads

Another way to obtain a running version is
to just download the latest stable or development release of OpenGeoSys from the [Releases](/releases)-page.

By downloading from the release page you will get a bunch of folders and files. However, OGS itself will come as a simple
executable file, which you will find in the `bin` sub-folder. You can put the executable wherever you like. For convenience you
may put it into a location which is in your `PATH`-environment variable which allows you to start the executable without
specifying its full file path just calling `ogs` from the terminal of your machine.

</div>

</div>

<div class='linux'>

</div>

<div class='mac'>

</div>

## Download benchmarks

You can download the latest benchmark files from GitLab:

- On our OGS repository on GitLab browse to the [Tests/Data](https://gitlab.opengeosys.org/ogs/ogs/-/tree/master/Tests/Data)
-folder
- Browse to the process subfolder you are interested in, e.g. [Elliptic](https://gitlab.opengeosys.org/ogs/ogs/-/tree/master/Tests/Data/Elliptic) (1.)
- Find the Downloads-dropdown (2.)
- Choose an appropriate format under **Download this directory** (3.)
- Uncompress the downloaded file

See [the Benchmarks section](/docs/benchmarks/) for more information on the benchmarks.

![Instructions for downloading benchmarks](/docs/userguide/basics/Download_Benchmarks.png)

## Running

OGS is a command line application and requires the path to a `.prj`-file as an argument.

<div class='win'>

To run it open a new command line shell (called *cmd.exe*). Now simply type `ogs` (if the executable is in your `PATH`
-environment variable) or specify its full path (e.g.: `C:\Users\MyUserName\Downloads\ogs.exe`) and hit `ENTER`.

OGS prints out its usage instructions:

```bash
PARSE ERROR:
             Required argument missing: project-file

Brief USAGE:
   ogs  [--] [--version] [-h] <PROJECT FILE>

For complete USAGE and HELP type:
   ogs --help
```

You can see that there is the project-file missing.

Then simply supply the path to a project file as an argument to the OGS executable:

```bash
ogs .\Path\to\BenchmarkName.prj
```

</div>

<div class='linux'>

To run it open a new command line shell (*Terminal*). Now simply type `ogs` (if the executable is in your `PATH`-environment
variable) or specify its full path (e.g.: `./path/to/ogs`) and hit `ENTER`.

OGS prints out its usage instructions:

```bash
PARSE ERROR:
             Required argument missing: project-file

Brief USAGE:
   ogs  [--] [--version] [-h] <PROJECT FILE>

For complete USAGE and HELP type:
   ogs --help
```

You can see that there is the project-file missing.

Then simply supply the path to a project file as an argument to the OGS executable:

```bash
ogs ./path/to/BenchmarkName.prj
```

</div>

<div class='mac'>

To run it open a new command line shell (*Terminal*). Now simply type `ogs` (if the executable is in your `PATH`-environment
variable) or specify its full path (e.g.: `./path/to/ogs`) and hit `ENTER`.

OGS prints out its usage instructions:

```bash
PARSE ERROR:
             Required argument missing: project-file

Brief USAGE:
   ogs  [--] [--version] [-h] <PROJECT FILE>

For complete USAGE and HELP type:
   ogs --help
```

You can see that there is the project-file missing.

Then simply supply the path to a project file as an argument to the OGS executable:

```bash
ogs ./path/to/BenchmarkName.prj
```

</div>
