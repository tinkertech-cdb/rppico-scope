# Hosting

## Primary Forge

- Codeberg only
- Canonical repository: `https://codeberg.org/tinkertech-cdb/rppico-scope`

## Policy

- Issues, pull requests, releases, and project planning are managed on Codeberg.
- No mirror to other forges during MVP unless explicitly enabled later.

## Local Git Workflow

- `origin` is Codeberg and is the default push target.
- `github` is optional mirror remote.
- Canonical development flow:
  1. `git push` (publishes to Codeberg)
  2. `git push-github-mirror` (publishes `main` to `github/codeberg-main`)
