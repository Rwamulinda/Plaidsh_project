#! /usr/bin/env python3
#
# An automated interactive test for plaidsh, based on top of pexpect
#
# Author: Howdy Pierce, howdy@sleepymoose.net
#
#  Usage: ./ps_interactive_test.py (executable-name)

import pexpect
import sys
import re
import os
from pathlib import Path

# re to match the prompt
prompt_re = r"#\?.* "

initial_cwd=os.getcwd()

# find the setup_playground script by searching these directories in order
script_path = [Path(__file__).parent, Path.cwd(), Path("/var/local/isse-12")]
setup_script = None

for s in script_path:
    setup_script = s / "setup_playground.sh"
    if setup_script.exists():
        break

if setup_script is None or not setup_script.exists():
    print("setup_playground.sh not found")
    sys.exit(1)

setup_script = str(setup_script.resolve())
# print(f"Found {setup_script}...")

# the variable a_file will be filled in to some file in the cwd that
# does not begin with a .
a_file=None
for f in os.listdir("."):
    if (f[0] != '.'):
        a_file = f
        break

if (a_file is None):
    print("No files found in cwd...")
    sys.exit(1)

#
# The list below is of the form
#    (input-string) (expected-output-regexp) (expect-empty) (points)
#
# Note that the input-string is subject to Python escaping rules
# before being passed to the shell. For instance, in the test case
# that says
#    "cd Plaid\\ Shell\\ Playground"
# the string actually passed will be:
#    cd Plaid\ Shell\ Playground
#
# Likewise, the expected-output-regexp is a _regular_expression_ that
# must match some portion of the output of each command. This is also
# subject to Python escaping rules.
#
# If expect-nothing-else is True, then we don't expect any additional
# output not consumed by expected-output-regexp

tests = [
    # forms of the empty string
    ("", "", True, 1),
    ("    ", "", True, 1),
    ("  \\  ", "Command not found|No such file", False, 1),

    # examples from p. 1 of the assignment writeup
    ("pwd", initial_cwd, True, 1),
    ("ls --color", a_file, False, 1),
    (setup_script, "Plaid Shell playground created", True, 1),
    (r"cd Plaid\ Shell\ Playground", "", True, 1),
    ("pwd", f"{initial_cwd}/Plaid Shell Playground", True, 1),
    ("ls",
     "README +'best sitcoms.txt'[ \t]+'seven dwarfs.txt'[ \t]+shells.txt",
     True, 1),
    ("ls *.txt",
     "'best sitcoms.txt'[ \t]+'seven dwarfs.txt'[ \t]+shells.txt",
     True, 1),
    ("echo $PATH", "\\$PATH", True, 1),
    ("author", "", False, 1),
    ("author | sed -e \"s/^/Written by /\"", "Written by ", False, 1),
    ("grep Happy *.txt",
     "best sitcoms.txt:Happy Days.*seven dwarfs.txt:Happy", True, 1),
    ("cat \"best sitcoms.txt\" | grep Seinfeld", "Seinfeld", True, 1),
    ("cat \"best sitcoms.txt\"|grep Seinfeld|wc -l", "1", True, 1),
    ("sed -ne \"s/The Simpsons/I Love Lucy/p\" < best\\ sitcoms.txt > output",
     "", True, 2),
    ("ls -l", "output", False, 1),
    ("cat output", "I Love Lucy", True, 1),
    ("this is not a command", "Command not found|No such file", False, 2),
    ("echo Hello > /usr/bin/cant_write", "Permission denied", True, 2),
    ("cd", "", True, 1),
    ("pwd", os.getenv("HOME"), True, 2),
    (f"cd {initial_cwd}", "", True, 1),
    ("pwd", initial_cwd, True, 1),
    ("cd ~", "", True, 1),
    ("pwd", os.getenv("HOME"), True, 2),

    # Test the up-arrow for history
    ("echo \"Operator could you help me place this call?\"",
     "Operator could you help me place this call\\?", True, 1),
    ("seq 10 | wc\"-l\"", "10", True, 1),
    ('\x1b[A\x1b[A', "Operator could you help me place this call\\?", True, 2),

    # additional tricky examples, some with errors
    ("env\t|grep PATH", os.getenv("PATH"), True, 2),
    ("echo Hello World | cat -n | cat -n | cat -n",
     "1[ \t]+1[ \t]+1[ \t]+Hello World", True, 2),
    ("echo \\c", "Illegal escape character '?c'?", True, 2),
    ("echo \"\\c\"", "Illegal escape character '?c'?", True, 1),
    ("echo \"hi", "Unterminated quote", True, 2),
    ("echo \"|1|2|3|\" | sed -e \"s/[0-9]//g\"", r"\|\|\|\|", True, 1),
    ("printf \"=%s=\\n\" one two three four five six seven eight nine ten "
     "eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen "
     "nineteen twenty twenty-one twenty-two twenty-three twenty-four|"
     "wc -l", "24", True, 2),
    ("printf\"+%s\"one\"two\"three\n", "\\+one\\+two\\+three", True, 2),
    ("echo > file1 >file2", "Multiple redirection", True, 1),
    ("cat <", "Expect filename after", False, 1),
    ("cat | cat | cat >", "Expect filename after", False, 1),
    ("grep | ", "No command (specified|found)", True, 1),
    ("| grep", "No command (specified|found)", True, 1),
    ("echo || grep", "No command (specified|found)", True, 1),
    ("echo \\<\\|\\> | cat", "<\\|>", True, 1),
    ("echo hello\\|grep ell", "hello\\|grep ell", True, 1)
]

def filter(line):
    # From https://stackoverflow.com/questions/14693701/how-can-i-remove-the-ansi-escape-sequences-from-a-string-in-python
    ansi_escape = re.compile(r'(?:\x1B[@-_]|[\x80-\x9F])[0-?]*[ -/]*[@-~]')
    return ansi_escape.sub('', line).replace('\r','').replace('\n','')

# run the tests and report the resulting score
def run_tests(executable):
    score_pts=0    # points scored
    total_pts=0    # total possible points

    prev_inp=""

    child = pexpect.spawn(executable, encoding='utf-8', timeout=1)

    logname = f"{executable}_test.log"
    flog = open(logname, mode='w')
    child.logfile_read = flog

    # Advance past any welcome text to the first prompt
    try:
        child.expect(prompt_re)
    except (pexpect.EOF, pexpect.TIMEOUT):
        print(f"FAIL: No prompt")

    for test in tests:
        inp = test[0]
        exp_result = test[1]
        expect_empty = test[2]
        total_pts += test[3]
        success = False

        # send input
        child.sendline(inp)

        # our input is echoed back to us, so advance past the input line
        child.expect(r'\r\n')

        # look for expected result
        try:
          child.expect(exp_result)
          success = True
        except pexpect.TIMEOUT:
          print(f"FAIL: Input '{inp}': Expected '{exp_result}'")
        except pexpect.EOF:
          print(f"FAIL (EOF): Input '{inp}': Expected '{exp_result}'")

        # look for next prompt 
        try:
            child.expect(prompt_re)
        except (pexpect.EOF, pexpect.TIMEOUT):
            print(f"FAIL: No prompt")

        # ensure there was no unexpected output prior to next prompt
        if success and expect_empty:
            if filter(child.before) != "":
                print(f"FAIL: Unexpected output '{filter(child.before)}' with input '{inp}'")
                success = False

        if success:
            score_pts += test[3]

        prev_inp=inp


    # make certain that exit works
    child.sendline("exit")
    try:
        child.expect(pexpect.EOF)
    except pexpect.TIMEOUT:
        print("FAIL: Program did not exit as expected")

    # check exit status
    child.close()

    total_pts += 2
    if (child.exitstatus == 0):
        score_pts += 2
    else:
        print(f"FAIL: plaidsh exited with status {child.exitstatus}")

    # check for memory leaks
    total_pts += 5
    if (child.before.find("memory leak") < 0):
        score_pts += 5
    else:
        print("FAIL: Memory leaks detected")

    print(f"Raw Score: {score_pts} out of {total_pts}")
    if (score_pts == total_pts):
        print("  ...plus 5 bonus points for passing all the tests!") #
        print(f"Total score: {score_pts+5} out of {total_pts+5}")
    else:
        print("TEST FAILURES EXIST")
        print(f"Total score: {score_pts} out of {total_pts+5}")


if (len(sys.argv) != 2):
    print(f"Usage: {sys.argv[0]} (executable-name)")
    sys.exit(1)

run_tests(sys.argv[1])
