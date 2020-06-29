# github-issues

A bash script that uses the GitHub v3 REST API, curl, grep and sed to find 
the number of open issues on a given repository, then print that to `stdout`.

# Dependencies

- `curl`
- `grep`
- `sed`

# Usage

    github-issues user repo

Example:

    github-issues domsson succade 
