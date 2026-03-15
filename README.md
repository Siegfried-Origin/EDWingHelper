![banner](https://repository-images.githubusercontent.com/1178061612/33a3cdaf-9e38-4899-910d-f5b091d3111e)

# ED Wing Helper

Small utility to help create a shared instance in Elite Dangerous by managing wing invitations from a predefined commander list.

The tool monitors the game journal to detect when commanders are online and when they join the wing, helping the wing leader keep track of who still needs to be invited.

⚠️ This project is currently in a very early stage and largely untested. Expect bugs.

![screenshot](https://github.com/user-attachments/assets/7c424738-fae6-4e30-aaac-f2ef1b2af3bb)
## How it works

1. Prepare a text file containing commander names (one name per line).
2. Import the file using the **"Import List..."** button.
3. Ensure all listed commanders are in your **friend list** in-game or an friend request was sent to them.
4. The tool monitors the Elite Dangerous **journal files** to track:
   - which commanders are **online**
   - which commanders have **joined the wing**

Commanders are automatically categorized in the UI:

- **Online – waiting for invite**
- **Offline – waiting for invite**
- **Already invited**

If a commander who was already invited **goes offline**, they are automatically removed from the *Already invited* section.
If a commander accepts your friend request while you're building the instance, it's status will be updated.


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
