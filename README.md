# ED Wing Helper

Small utility to help create a shared instance in Elite Dangerous by managing wing invitations from a predefined commander list.

The tool monitors the game journal to detect when commanders are online and when they join the wing, helping the wing leader keep track of who still needs to be invited.

⚠️ This project is currently in a very early stage and largely untested. Expect bugs.

## Overview

### Presentation video
[![Presentation video](https://img.youtube.com/vi/mwUawYZSdxg/0.jpg)](https://www.youtube.com/watch?v=mwUawYZSdxg)

### Main interface
![screenshot](https://github.com/user-attachments/assets/7c424738-fae6-4e30-aaac-f2ef1b2af3bb)

## How it works

1. Prepare a text file containing commander names (one name per line).
2. Import the file using the **"Import List..."** button.
3. Ensure all listed commanders are in your **friend list** in-game or an friend request was sent to them.
4. The tool monitors the Elite Dangerous **journal files** to track:
   - which commanders are **online**
   - which commanders have **joined the wing**

Commanders are automatically categorized in the UI:

- **Need Invite**
  - Online (waiting your wing invite)
  - Offline
- **Instanced**
   - Confirmed (they were dected in the local chat)
   - Unconfirmed (they've accepted your wing request)

The program automatically move commanders between sections:
- If a commander who was already invited **goes offline**, they are automatically removed from the *Instanced* section. However, sometimes, the journal does not report Offline events or the even may be delayed.
- If a commander accepts your friend request while you're building the instance, its status will be updated.
- If a commander of your list speaks in the local chat channel, they are moved to the *Instanced* section: you don't need to invite them, they're already in the instance
- If a commander have joined your wing and speaks in the local chat channel, they are moved from *Unconfirmed* to *Confirmed*. This helps you sort between commander that have left your wing but did not successfully joined your instance.

## Requirements

- Elite Dangerous must be running
- The game must have **journal logging enabled** (enabled by default)
- You must be friend with a commanders to invite them in the wing


## Planned features

The following improvements are planned:

### List management
- Drag & drop a file to **replace or append** the commander list
- Monitor the list file and **auto-reload when modified**

### Monitoring improvements
- Detect commanders **already present in the instance** via local chat monitoring

### Wing management
- Handle **wing leader disconnects / leaving the wing** (FSD jump, crashes, etc.)
- Recover the **invited list after program restart**


## Project status

This tool is experimental and currently under active development.
Features and behavior may change frequently.
