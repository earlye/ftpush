ftpush
------

A command line utility for pushing files to an FTP server.
This is, in theory, useful for making a remote web site a mirror of
a local directory.

```
Usage: ftpush [options]
Available Options:
  -h [ --help ]         produce help message
  -v [ --version ]      display version
  --site arg            input file
```

Given an input file, ftpush will upload a site to a remote server.

The input file format is boost ini compatible.  Here's a template to start with:

```ini
[site]
host={host}
username={username}
password={password}
time-cache={filename}

[log]
transfers=0
skipped-files=0
ignore-locals=0
ftp-commands=0
time-check=0
uploading-files=0

[directory-map]
{local-path}={remote-path}

[ignore-local]
{regex}=1
```

