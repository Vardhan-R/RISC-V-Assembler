make
.\main.exe $args[0]
py "risc-v_parser.py" $args[0]
make "clean"
Remove-Item "py_output.txt"
