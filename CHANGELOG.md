# Changelog

## 1.1.0

API changes:

- Adds pulse counter support
- XXXInit returns failure if called more than once without XXXDeinit
- Deprecates `--username` argument for CLI tools. See "CLI tools cache a token" below

Improvements:

- CLI tools cache a token and so request username and password less frequently
- Adds support to skip GNSS fix for test
- Fixes issue where GPIO events generated during job execution may get missed
- Adds message_inject.py tool to programatically send test messages
- Adds better support/documentation for testing in an indoor lab environment
- Adds support to build applications offline
- Consolidates header files and cleans up inline documents

## 1.0.0

Initial release
