# Release Pipeline

## Trigger (API call)

The workflow uses `workflow_dispatch`, so it can be started from the GitHub REST
API. The single input `bump` selects which version element to increase:
`major`, `minor` or `patch`.

> **Prerequisite:** `release.yml` must already exist on the default branch
> (`master`) before the API trigger is available.

### REST endpoint

```
POST /repos/lucapf/rl_for_dummies/actions/workflows/release.yml/dispatches
```

Headers:

```
Authorization: Bearer <TOKEN>          # PAT with 'repo' (classic) or Actions:write (fine-grained)
Accept: application/vnd.github+json
X-GitHub-Api-Version: 2022-11-28
```

Body:

```json
{ "ref": "master", "inputs": { "bump": "patch" } }
```

### curl

```bash
curl -X POST \
  -H "Authorization: Bearer $GH_TOKEN" \
  -H "Accept: application/vnd.github+json" \
  https://api.github.com/repos/lucapf/rl_for_dummies/actions/workflows/release.yml/dispatches \
  -d '{"ref":"master","inputs":{"bump":"patch"}}'
```

### gh CLI (equivalent)

```bash
gh workflow run release.yml -f bump=patch     # or bump=minor / bump=major
```

## What the pipeline does

1. **Compute version** — reads the latest git tag (if no tags use 0.0.0). 
   increase the last tag according the bump.

2. **Compile**. If any warning / error, the release abort.

4. **Tag**

5. **Create a new release**. Upload the file into the release

## The artifact will be available at the address

   ```
   https://github.com/lucapf/rl_for_dummies/releases/download/<version>/tris-<version>
   ```

   `https://github.com/lucapf/rl_for_dummies/releases/latest` always resolves to
   the most recent release.
