# Malware Laboratory 1 — Rootkit Creation

**Course:** Malware  
**Authors:** Benjamin Olsen, Lluís Vila

---

## Exercise 1 — Process Hiding

### 1.1 Objective

The goal of this exercise is to develop a library that can hide a specific process from the `ps` command. To achieve this, we use `LD_PRELOAD`, which allows us to intercept functions from dynamic libraries. In this case, we intercept `readdir()`, which is used to read directory entries in Linux.

The library identifies the target PID and skips its entry while reading the `/proc` directory. In this way, the process does not appear in the `ps` output when the library is loaded.

### 1.2 Implementation

For the implementation, we used the provided base code.

**Identification of the `/proc` directory**

We implemented the function `is_proc()`, which uses `dirfd()`, `fstat()` and `stat()` to obtain and compare the inode numbers of the current directory and `/proc`. This check helps prevent hiding files that are not inside `/proc`.

```c
int is_proc(DIR *dirp) {
    struct stat a, b;
    fstat(dirfd(dirp), &a);
    stat("/proc", &b);
    return a.st_ino == b.st_ino;
}
```

**PID configuration**

We implemented three ways to select the target process: a hardcoded PID (`174618`), an environment variable (`PID`) and a configuration file (`/tmp/lib.cfg`).

A priority system is applied: the configuration file has the highest priority, followed by the environment variable and finally the hardcoded PID.

**Process filtering**

Within the intercepted `readdir()` function, the name of the retrieved entry is compared with the configured PID using `strcmp()`. If the entry belongs to `/proc` and matches that PID, the next entry is requested and returned instead.

### 1.3 Testing and Results

To verify the correct operation of `lib1.so`, we performed two different tests.

**Process hiding using an environment variable**

In the first test, we created a `sleep 300` process and obtained its PID (`346702`). Then, we executed `ps` with the library loaded through `LD_PRELOAD`, using the previously identified PID as an environment variable.

As shown in Figure 1, the process does not appear in the list when the library is used. However, when we execute `ps` normally, the process appears in the terminal. This confirms that the process is still active and that the library only affects its visibility in the command output.

![Process hiding using LD_PRELOAD](images/lib1_1.png)

*Figure 1. Comparison of ps output with and without LD_PRELOAD, showing the hiding of PID 346702.*

**Configuration file priority**

In the second test, we created another `sleep 300` process with PID `350009` and saved this value in `/tmp/lib.cfg`.

Then, we executed `ps` with the library loaded and also set the environment variable `PID=349870`, which is different from the PID stored in the file.

As shown in Figure 2, the library hides process `350009`. This confirms that the configuration file has higher priority than the environment variable. When we execute `ps` normally, the process appears again.

![Configuration file priority](images/lib1_2.png)

*Figure 2. Verification that /tmp/lib.cfg takes precedence over the PID environment variable.*

### 1.4 Difficulties and Solutions

One of the main challenges was avoiding false positives. Since `readdir()` is used to read many directories, filtering only by PID could also hide normal files with the same name. To reduce this problem, we implemented `is_proc()`, which checks whether the directory being read is `/proc` by comparing inode numbers.

---

## Exercise 2

---

## Exercise 3 — In-Memory File Content Spoofing

### 3.1 Objective

### 3.2 Implementation

### 3.3 Testing and Results

### 3.4 Difficulties and Solutions

---

## AI Usage Declaration
