# PACE-MAF-2026

Solution for the PACE 2026 Challenge.

## Requirements

To build the project you need `g++` with C++17 support, `make`, and IBM CPLEX / Concert installed locally.

## Build

```sh
make
```

This generates `./pace_maf`.

To clean the project:

```sh
make clean
```

## Run The Solver

Run all dataset instances:

```sh
make run-all
```

Run the debug target:

```sh
make debug
```

Run the test instances:

```sh
make test
```

Run one instance manually:

```sh
./pace_maf input/dataset/tiny01.nw
```

Solutions are written to:

```sh
output/out_<instance-name>.nw
```

## Run The Checker

Check all instances in an input folder:

```sh
./checker.sh dataset
./checker.sh test
```

Check one instance manually:

```sh
./checker input/dataset/tiny01.nw output/out_tiny01.nw
```

## Typical Workflow

```sh
make
make run-all
./checker.sh dataset
```
