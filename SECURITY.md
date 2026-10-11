# Security policy for the comninos22 fork

The initial reviewed release is `v1.1.93-security.1`. Review scope and remaining limitations are recorded
in [docs/security-review.md](docs/security-review.md). No support response time is promised.

Report suspected vulnerabilities through this fork's
[GitHub security page](https://github.com/comninos22/paint.software/security/advisories/new).
Do not include secrets or weaponized exploit payloads in public issues. If private reporting is unavailable,
use a minimal public request for a private contact method without disclosing vulnerability details.

Keep system Qt and image codecs patched. Native plugins are disabled by default and run with full user
permissions if explicitly enabled. This program does not provide a sandbox for untrusted plugins or codecs.
The review does not establish that upstream authors intended malicious behavior.
