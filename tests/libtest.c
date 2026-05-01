/*
 *   Copyright 2025-2026 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <shared/str.h>
#include <tests/base.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/alloc/base.h>

#include <valgrind/valgrind.h>

#include <elf.h>
#include <gelf.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <libelf.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


bool test_is_on_valgrind;
int tty_fd = 1;
bool has_file = false;
char* test_name = NULL;


void
test_say_common(
	const char* format,
	bool important,
	va_list args
	)
{
	char buffer[4096];
	sprintf(buffer, important ?
		"\033[1m\033[35m[%s]\033[39m > %s <\033[0m\n" :
		"\033[1m\033[35m[%s]\033[39m\033[0m %s\n",
		test_is_on_valgrind ? "VEST" : "TEST", format);

	va_list copy;
	va_copy(copy, args);
		vdprintf(tty_fd, buffer, copy);
	va_end(copy);

	if(has_file)
	{
		int len = strlen(buffer);
		int new_len = 0;

		for(int i = 0; i < len; ++i)
		{
			if(buffer[i] == '\033' && buffer[i + 1] == '[')
			{
				while(buffer[i] != 'm' && i < len) ++i;
			}
			else
			{
				buffer[new_len++] = buffer[i];
			}
		}

		buffer[new_len] = '\0';

		vfprintf(stderr, buffer, args);
	}
}


void
test_say(
	const char* format,
	...
	)
{
	va_list args;
	va_start(args, format);
		test_say_common(format, false, args);
	va_end(args);
}


void
test_shout(
	const char* format,
	...
	)
{
	va_list args;
	va_start(args, format);
		test_say_common(format, true, args);
	va_end(args);
}


void
test_set_timeout(
	unsigned int seconds
	)
{
	alarm(seconds);
}


typedef struct test
{
	str_t name;
	bool should_pass;
	bool should_timeout;
	int pid;
}
test_t;

test_t* tests = NULL;
uint32_t tests_count = 0;
uint32_t tests_ran = 0;
uint32_t tests_passed = 0;
uint32_t tests_failed = 0;


void
wait_and_run_tests(
	void
	)
{
	int status;
	test_t test;
	test.pid = -1;

	if(!test_name)
	{
		int pid = waitpid(-1, &status, 0);
		assert_neq(pid, -1);

		for(uint32_t j = 0; j < tests_count; ++j)
		{
			if(tests[j].pid == pid)
			{
				test = tests[j];
				break;
			}
		}
	}
	else
	{
		status = 0;
		test = tests[0];
	}

	assert_neq(test.pid, -1);

	bool success = WIFEXITED(status) && WEXITSTATUS(status) == 0 && test.should_pass;
	success |= WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT && !test.should_pass && !test.should_timeout;
	success |= WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM && test.should_timeout;

	if(success)
	{
		test_say("%-60s passed", (char*) test.name->str);
		++tests_passed;
	}
	else
	{
		if(WIFSIGNALED(status))
		{
			test_shout("\033[31m%-58s failed: aborted with signal %s\033[39m",
				(char*) test.name->str, sigabbrev_np(WTERMSIG(status)));
		}
		else if(WIFEXITED(status))
		{
			test_shout("\033[31m%-58s failed: exited with status %d\033[39m",
				(char*) test.name->str, WEXITSTATUS(status));
		}
		else if(WIFSTOPPED(status))
		{
			test_shout("\033[31m%-58s failed: stopped with signal %s\033[39m",
				(char*) test.name->str, sigabbrev_np(WSTOPSIG(status)));
		}
		else
		{
			test_shout("\033[31m%-58s failed: returned unknown status %d\033[39m",
				(char*) test.name->str, status);
		}

		++tests_failed;
	}

	str_free(test.name);
	++tests_ran;
}


int
main(
	int argc,
	char** argv
	)
{
	test_is_on_valgrind = RUNNING_ON_VALGRIND;

	tty_fd = open("/dev/tty", O_WRONLY);
	if(tty_fd == -1)
	{
		tty_fd = STDERR_FILENO;
	}

	int num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
	int max_concurrent_tests = MACRO_MAX(1, num_cpus / 2);

	for(int i = 1; i < argc; ++i)
	{
		if(!strcmp(argv[i], "--file"))
		{
			has_file = true;
		}
		else if(!strcmp(argv[i], "--name"))
		{
			if(i + 1 < argc)
			{
				test_name = argv[++i];

				char prio[16];
				char method[16];
				char name[256];
				int len = sscanf(test_name, "test_%15[^_]_%15[^_]__%255[^(]", prio, method, name);
				if(len == 3)
				{
					test_name += 5 + strlen(prio) + 1 + strlen(method) + 2;
				}
			}
			else
			{
				test_shout("Missing argument for --name");
				return 1;
			}
		}
		else if(!strcmp(argv[i], "--max"))
		{
			if(i + 1 < argc)
			{
				max_concurrent_tests = atoi(argv[++i]);
			}
			else
			{
				test_shout("Missing argument for --max");
				return 1;
			}
		}
		else
		{
			test_shout("Unknown argument '%s'", argv[i]);
			return 1;
		}
	}

	void* handle = dlopen(NULL, RTLD_LAZY);
	assert_not_null(handle);

	int fd = open("/proc/self/exe", O_RDONLY);
	assert_neq(fd, -1);

	assert_neq(elf_version(EV_CURRENT), EV_NONE);

	Elf* elf = elf_begin(fd, ELF_C_READ, NULL);
	assert_not_null(elf);

	GElf_Ehdr ehdr;
	assert_not_null(gelf_getehdr(elf, &ehdr));

	Elf_Scn* scn = NULL;
	GElf_Shdr shdr;

	while((scn = elf_nextscn(elf, scn)) != NULL)
	{
		if(gelf_getshdr(scn, &shdr) == NULL) continue;

		if(shdr.sh_type == SHT_SYMTAB)
		{
			Elf_Data* data = elf_getdata(scn, NULL);
			assert_not_null(data);

			int num_symbols = shdr.sh_size / shdr.sh_entsize;
			for(int i = 0; i < num_symbols; ++i)
			{
				GElf_Sym symbol;
				if(gelf_getsym(data, i, &symbol) == NULL) continue;

				const char* sym_name = elf_strptr(elf, shdr.sh_link, symbol.st_name);
				char prio[16];
				char method[16];
				char name[256];
				int len = sscanf(sym_name, "test_%15[^_]_%15[^_]__%255[^(]", prio, method, name);
				if(len != 3) continue;

				if(
					strcmp(prio, "normal") &&
					strcmp(prio, "priority")
					)
				{
					test_say("Invalid test priority '%s'", prio);
					continue;
				}

				bool priority = !strcmp(prio, "priority");

				if(
					strcmp(method, "fail") &&
					strcmp(method, "pass") &&
					strcmp(method, "timeout")
					)
				{
					test_say("Invalid test method '%s'", method);
					continue;
				}

				if(test_name != NULL && strcmp(name, test_name))
				{
					continue;
				}

				assert_eq(symbol.st_info, ELF32_ST_INFO(STB_GLOBAL, STT_FUNC));

				while(tests_count - tests_ran >= max_concurrent_tests)
				{
					wait_and_run_tests();
				}

				bool should_pass = !strcmp(method, "pass");
				bool should_timeout = !strcmp(method, "timeout");

				void (*test_func)() = dlsym(handle, sym_name);
				assert_not_null(test_func);

				test_say("%-60s expecting %s", name, should_pass ? "success" : should_timeout ? "timeout" : "failure");

				int pid;
				if(!test_name)
				{
					pid = fork();
					assert_neq(pid, -1);
				}
				else
				{
					pid = 0;
				}

				if(pid == 0)
				{
					nice(priority ? 5 : 20);

					alloc_reset();
					test_set_timeout(60);

					test_func();

					if(!test_name)
					{
						exit(0);
					}
				}

				tests = alloc_remalloc(tests, tests_count, tests_count + 1);
				assert_not_null(tests);

				str_t name_str = str_init_copy_cstr(name);

				tests[tests_count++] =
				(test_t)
				{
					.name = name_str,
					.should_pass = should_pass,
					.should_timeout = should_timeout,
					.pid = pid
				};

				if(test_name)
				{
					break;
				}
			}
		}
	}

	elf_end(elf);
	close(fd);
	dlclose(handle);

	uint32_t left = tests_count - tests_ran;
	for(uint32_t i = 0; i < left; ++i)
	{
		wait_and_run_tests();
	}

	test_shout("Ran %d tests, \033[32m%d\033[39m passed, \033[31m%d\033[39m failed",
		tests_count, tests_passed, tests_failed);

	alloc_free(tests, tests_count);

	close(tty_fd);

	return !!tests_failed;
}
