# PACE-MAF-2026

Solution for the PACE 2026 Challenge.

## Makefile

```sh
make
make clean
make run-all
make debug
make test
./paceSolver input/dataset/tiny01.nw
```
1. Compile generating `paceSolver`
2. Removes Build
3. Executes all files in input/dataset
4. Executes only one file of input/dataset
5. Executes all files in input/test
6. Manual execution

## Run The Checker

Check all instances in an input folder:

```sh
./checker.sh dataset
./checker.sh test
./checker input/dataset/tiny01.nw output/out_tiny01.nw
```

## Typical Workflow

```sh
make
make run-all
./checker.sh dataset
```
## Stride

#### Steps:
1. Download list_name.lst from [link](https://pace2026.imada.sdu.dk)
2. Put the file in `input/dataset/lists`
3. Run strideRunner.sh list_name

## Memory Leaks

Check leaks with
```sh
leaks --atExit -- ./paceSolver < input/test/test03.nw > outLeak.txt
```