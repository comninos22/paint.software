# Security review: comninos22/paint.software

Reviewed upstream: `Univers4craft/paint.software` at `4cb2bb87ca9bed4e8fc1816b8ba385fc3eabe204`.
Review date: 2026-10-11. Scope: source, native file format, image loading, plugins, build scripts,
packaging and release workflows at that revision, with targeted history inspection.

## Assessment

No evidence of an intentional backdoor, credential theft, persistence mechanism, or application-level
network exfiltration was found in the reviewed source. The application has no Qt Network dependency.
The process launch found in application code invokes `gsettings` with fixed arguments to query desktop
appearance; the plugin loader intentionally executes native code. Neither observation proves malicious
intent. This is a bounded review, not a guarantee of safety or an exhaustive audit of every historical commit.
Previously published upstream binaries were not downloaded or verified against source.

## Findings and changes

| Finding | Assessment | Change in this fork |
| --- | --- | --- |
| Native `.psw` dimensions and aggregate layer memory are not bounded; QString lengths are deserialized before validation | Crafted-file resource exhaustion risk; the original limit of 4,096 layers is not a practical memory budget | Validate encoded file size, dimensions, pixel budget, layer count and name length before allocation; check embedded PNG headers |
| Native format version, active index, enum, opacity, offsets and layer/canvas consistency are insufficiently validated | Confirmed acceptance of invalid metadata and signed-integer overflow during rendering with a small crafted file; arbitrary code execution has not been established | Reject unsupported versions, invalid indices/booleans/blends, NaN/infinity, excessive offsets, mismatched dimensions, truncation and trailing data; preserve the current document on rejection |
| AppImage workflow selects Qt 6.5.3 and qtimageformats | Qt 6.5.3 is within the affected versions of the published ICNS crash vulnerability CVE-2025-5683; this is a dependency finding, not evidence of intent | Retire this AppImage pipeline; ship a distribution-linked `.deb`, and restrict Open/Import to the advertised raster formats |
| AppImage tools are downloaded from mutable `continuous` URLs without integrity pins; aqtinstall and Actions use mutable references | Build supply-chain exposure, with no compromise demonstrated | Remove external AppImage downloads; pin remaining Actions to exact commits; use distribution package repositories |
| APT publishing falls back to unsigned metadata and recommends `trusted=yes` if signing secrets are absent | Integrity bypass by design in the fallback configuration; upstream's live APT deployment was not tested | Remove APT deployment and unsigned install instructions from active fork documentation |
| Flatpak manifest fetches upstream `main` independently of the checked-out revision, using an old runtime and broad home access | Release provenance mismatch and unnecessary file access; a fork build could omit its own changes | Retire inherited Flatpak workflow/manifests; the supported release builds the reviewed local source |
| Native plugins are automatically loaded at startup | Documented arbitrary native-code execution by design, not by itself a vulnerability | Disable startup loading unless `PAINTSW_ENABLE_PLUGINS=1`; document the trust boundary |
| Saves write directly to destination files | Interrupted/failed saves can damage an existing document; reliability issue | Use QSaveFile replacement for native and raster saves |
| Publishing jobs have write permissions during builds, and can publish without tests | Release hardening gap | Use read-only workflows with persisted checkout credentials disabled; build/test before producing a package; publish releases separately |

Qt documents that QDataStream is not resilient to corrupted input and may allocate based on untrusted
serialized sizes: [QDataStream corruption and security](https://doc.qt.io/qt-6/qdatastream.html#corruption-and-security).
Qt's [CVE-2025-5683 advisory](https://www.qt.io/blog/security-advisory-recently-discovered-issue-in-icns-image-format-handling-impacts-qt)
identifies affected and fixed versions. This review did not claim that every Qt advisory affects this application.

## Verification

The repository includes a dedicated `security_regression` test covering valid Qt_6_0 native round trips,
invalid header fields and metadata, excessive dimensions/name lengths, layer count and aggregate budgets,
embedded PNG header validation, truncation, trailing data, failed-load state preservation, format filtering,
and failed-save preservation. The original functional and brush regression suites remain available.
Executed locally in an isolated Ubuntu 24.04 container, building/running as UID 65534:

- Release application and all test targets: built successfully with GCC 13.3 and Qt 6.4.2 distribution packages.
- CTest: **3/3 passed** (functional, brush_regression, security_regression).
- AddressSanitizer + UndefinedBehaviorSanitizer: **1/1 security suite passed**, with undefined behavior configured to halt the test. Leak detection was disabled; this is not a leak-cleanliness claim.
- Upstream baseline: four small invalid-metadata fixtures were accepted. Rendering a two-pixel-wide native layer with an extreme offset triggered a UBSan signed-integer-overflow diagnostic in `Layer::composited`; the hardened reader rejects it.
- The truncation tests initially exposed Qt accepting an incomplete PNG end checksum. A bounded PNG chunk walk and explicit end-checksum validation were added; all truncation cases then passed.
- Cppcheck 2.13 examined 97 implementation files with Qt library modeling. It reported three initialization-list performance suggestions and one uninitialized `m_after` warning. Manual inspection confirmed `m_after` is read only after `m_hasAfter` is set by `captureAfter`; these diagnostics did not establish another exploitable bug. System headers were not modeled by that scan.
- Offscreen application startup remained running until the intentional three-second smoke-test timeout.
- Debian package inspection confirmed the personal-fork homepage/maintainer, bundled MIT license, system Qt dependencies, and RELRO/BIND_NOW linker hardening.
- Workflow YAML and package shell syntax checks passed.

GitHub Actions could not start: the account is locked for an Actions billing issue. The failed hosted runs are infrastructure failures, not passing checks. The local commands above were actually executed. The repository restricts Actions to the exact pinned checkout/upload-artifact revisions and requires commit pinning.

Source inspection also checked process/network/file-loading entry points, plugin registration and sample
code, package maintainer scripts, and workflow triggers/permissions. A limited credential-pattern scan
found no private-key blocks, GitHub token patterns or AWS access-key patterns in tracked files. No tracked
symlinks or submodules were present. Workflow YAML and packaging shell syntax were checked locally.

## Limits and operational notes

- There was no attack against live infrastructure and no contact with upstream maintainers.
- This is not exhaustive fuzzing, formal verification, or a complete dependency/CVE audit.
- Qt and codec internals remain trusted native code; install vendor security updates. The `.deb` does not bundle Qt.
- Limits reduce resource-exhaustion exposure; they do not cap total process memory or CPU. Many open documents,
  editing/history operations, valid but expensive images, or allocation failures can still exhaust resources.
- Images enter through clipboard and UI operations too; the bounded decoder applies to disk Open/Import and
  the native file reader, not every in-memory image allocation.
- Explicitly enabled plugins retain full account access and can crash or compromise the process. They are not sandboxed.
- Retired upstream distribution instructions are preserved only in `docs/README.upstream.md` as historical context.
- Source history and the original MIT copyright notice are retained. This fork does not imply upstream endorsement.
