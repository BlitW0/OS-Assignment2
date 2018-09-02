shell: shell.c parser.c relative_path.c built_in.c ls.c pinfo.c process_launch.c clock_command.c
	gcc -o shell shell.c parser.c relative_path.c built_in.c ls.c pinfo.c process_launch.c clock_command.c -I .
