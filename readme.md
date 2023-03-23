# Dependencies
To run this on a calculator, the user must have a copy of a Commodore 64 Kernal, Basic ROM, and Character ROM, which must be converted to AppVars called `C64KERN`, `C64BASIC`, and `C64CHAR` using `convbin`

```bash
convbin -i BASIC.ROM -o C64BASIC.8xv -n C64BASIC -k 8xv
convbin -i KERN.ROM -o C64KERN.8xv -n C64KERN -k 8xv
convbin -i CHAR.ROM -o C64CHAR.8xv -n C64CHAR -k 8xv
```

# Building
Make sure you have the latest version of the toolchain.
```bash
make
```

If you want to see the traces on CEmu, make sure to build with the debug libraries
```bash
make debug
```

# License
This product is licensed under an MIT license