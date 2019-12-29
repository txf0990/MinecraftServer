#include <iostream>

using namespace std;

int main() {
  const int ROW_COUNT = 5;
  const int COL_COUNT = 5;
  int rx = 0;
  int cx = 0;
  while(true) {
    cout << "\033c";
    for (int r = 0; r < ROW_COUNT; r++) {
      for (int c = 0; c < COL_COUNT; c++) {
        if (r == rx && c == cx) {
          cout << '@';
        } else {
          cout << '.';
        }
      }
      cout << endl;
    }
    char key;
    cin >> key;
    if (!cin) break;
    switch (key) {
      case 'w':
        if (rx > 0) {
          --rx;
        }
        break;
      case 's':
        if (rx < ROW_COUNT - 1) {
          ++rx;
        }
        break;
      case 'a':
        if (cx > 0) {
          --cx;
        }
        break;
      case 'd':
        if (cx < COL_COUNT - 1) {
          ++cx;
        }
        break;
    }
  }
  return 0;
}
