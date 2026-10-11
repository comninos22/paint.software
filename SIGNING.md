# Release integrity

This fork publishes SHA-256 checksums alongside its source archive and Debian package.
Checksums detect corruption but are not an independent signature or proof of publisher identity.
Download from https://github.com/comninos22/paint.software/releases and inspect the tagged source and CI results.

There is no signed APT repository for this fork. The inherited APT publishing workflow and its unsigned
fallback have been removed. No signing keys are generated or copied from upstream.
