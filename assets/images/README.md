# Optional game icon images

Put transparent PNG icon images in this folder. Each file is loaded separately
when the game starts and drawn centered in its maze cell. Square images around
64 x 64 or 128 x 128 pixels work well; the game scales them to fit. Missing
images fall back to the existing raylib shapes.

Use these exact filenames:

| File | Used for |
| --- | --- |
| `player.png` | Player character |
| `enemy_patrol.png` | Blue patrol guard |
| `enemy_bfs.png` | Red BFS hunter |
| `enemy_astar.png` | Purple A* hunter |
| `powerup_speed.png` | Speed boost |
| `powerup_invisibility.png` | Invisibility |
| `powerup_freeze.png` | Enemy freeze |
| `powerup_health.png` | Health restore |
| `key.png` | Maze key |

Keep transparent padding around each silhouette so the icon does not touch the
cell walls. Player, enemy, power-up, and key images should be separate files;
the game does not require a sprite sheet.
