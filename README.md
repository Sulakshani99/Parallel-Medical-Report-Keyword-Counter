# Parallel-Medical-Report-Keyword-Counter

This project implements a parallel processing solution to count keywords in medical reports using different parallel computing approaches: Serial, OpenMP, MPI, and Hybrid (OpenMP + MPI).

## Project Structure
```
.
├── data/
│   └── medical_reports.txt    # Input medical reports
├── outputs/
│   ├── result_serial.txt     # Results from serial version
│   ├── result_openmp.txt     # Results from OpenMP version
│   ├── result_mpi.txt        # Results from MPI version
│   ├── result_hybrid.txt     # Results from hybrid version
│   └── performance.txt       # Performance Time
├── serial_version/
│   └── serial.c             # Serial implementation
├── openmp_version/
│   └── openmp.c             # OpenMP implementation
├── mpi_version/
│   └── mpi.c                # MPI implementation
├── hybrid_version/
│   └── hybrid.c             # Hybrid (OpenMP + MPI) implementation
└── keywords.txt             # List of keywords to search for
```

## Pre Requirements
- GCC compiler (gcc --version)
- OpenMP support (gcc -fopenmp --version)
- MPI implementation (e.g., OpenMPI ) (mpicc --version)
- Python 3.x (for running the scripts) (python3 --version)

## Installation

1. Install required packages:
```bash
# For Ubuntu
sudo apt-get update
sudo apt-get install gcc openmpi-bin libopenmpi-dev

2. Verify installation:
```bash
gcc --version
mpicc --version
```

## Running the Code

### 1. Serial Version
```bash
# Compile code
cd serial_version
gcc serial.c -o serial

# Run code
./serial
```

### 2. OpenMP Version
```bash
# Compile code
cd openmp_version
gcc -fopenmp openmp.c -o openmp

# Run code 
./openmp
```

### 3. MPI Version
```bash
# Compile code
cd mpi_version
mpicc mpi.c -o mpi

# Run code (using 4 processes)
mpirun -np 4 ./mpi
```

### 4. Hybrid Version
```bash
# Compile code
cd hybrid_version
mpicc -fopenmp hybrid.c -o hybrid

# Run code (using 2 MPI processes, each with 2 OpenMP threads)
export OMP_NUM_THREADS=2
mpirun -np 2 ./hybrid
```

