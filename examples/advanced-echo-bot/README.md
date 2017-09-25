## advanced-echo-bot

Echo bot that implementes per-channel switch.


Bot responds to every message it receives with following:
> Message ID: 357647131575713792<br>
> Channel ID: 342792472306384897<br>
> Sender ID: 130749397372764161<br>
>
> message text

| Environment Variable | Usage                              |
| -------------------- | ---------------------------------- |
| `BOT_TOKEN`          | Bot token.                         |
| `OWNER_ID`           | Who can run `echo-bot shutdown`.   |

| Commands             | Action                             |
| -------------------- | ---------------------------------- |
| `echo-bot turn-on`   | Turn on echoing for this channel.  |
| `echo-bot turn-off`  | Turn off echoing for this channel. |
| `echo-bot shutdown`  | Stop bot.                          |
