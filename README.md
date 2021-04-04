# M5Fire_HRSpO2
M5Stack + Mini Heart Rate Unit (MAX30100)で心拍数(HR)と酸素飽和度(SpO2)を計測/描画/記録  
M5Stack FIREで動作検証済み

# Main functions
- Wi-Fi 経由で日時を取得して表示
- HR/SpO2 を数値で表示
  - 取得周期は現状 10 [Hz]　（高速化可）
  - SpO2 (80% 以上) の経時変化をグラフ表示　（グラフ全体が流れるのではなく更新バーがローリング）
  - HR の経時変化をビープ音（音程とタイミング）で通知　（実装済みながら，HR/SpO2 の取得と干渉するため無効化）
- M5Core の左ボタンでビープ音を On/Off　（上記事情により無効化）
- M5Core の中ボタンで SD カードにログ（経過時間，イベント，SpO2，HR，csv 形式）出力を開始/停止
  - ファイル名は日時のため繰り返し計測しても重複しない
  - 記録中は Rec アイコンを表示
  - 書込不可（SD カード未挿入）の場合には反応しない
- M5Core の右ボタンで ログ記録中のイベント（HR/SpO2 に影響しそうな行動の開始/終了など）を記録．
  - イベント発生中は Active アイコンを表示
- GUI でのエラー表示（Wi-Fi 未接続，SD カード未挿入，センサ認識失敗など）
- バッテリ駆動により M5Core とセンサユニットだけで運用可
- コンパクトゆえに携帯/ウェアラブル機器として運用可

# Requirements
- [M5Stack用心拍センサユニット](https://www.switch-science.com/catalog/5695/)  
[Mini Heart Rate Unit (MAX30100) Pulse Oximeter](https://m5stack-store.myshopify.com/products/mini-heart-unit)
- [oxullo/Arduino-MAX30100 library](https://github.com/oxullo/Arduino-MAX30100)
- Wi-FiのSSIDとパスワードをinoファイルに記入
```
// Wi-Fi
char ssid[] = "SSID";
char pass[] = "PASSWORD";
```
- microSDカード（16GB以下）

# Licence
[GPLv3](https://github.com/komix-DIYer/M5Fire_HRSpO2/blob/main/LICENSE)

# Author
komix
