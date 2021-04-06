# M5Fire_HRSpO2
M5Stack + Mini Heart Rate Unit (MAX30100)で心拍数(HR)と酸素飽和度(SpO2)を計測/描画/記録  
M5Stack FIREで動作検証済み  
※計測の信頼度はセンサとライブラリの品質そのもの（本コードによる補正は無し）

[YouTube  
<img src="https://user-images.githubusercontent.com/25588291/113534902-e626ab80-960c-11eb-86e4-5cf3dbee893a.jpg" width="600">](https://www.youtube.com/watch?v=01Xj65EARtI)

# Main functions
- Wi-Fi 経由で日時を取得して表示
- HR/SpO2 を数値で表示
  - 取得周期は現状 10 [Hz]　（高速化可）
  - SpO2 (80% 以上) の経時変化をグラフ表示　（グラフ全体が流れるのではなく更新バーがローリング，下図 4～6）
  - HR の経時変化をビープ音（音程とタイミング）で通知　（実装済みながら，HR/SpO2 の取得と干渉するため無効化）
- M5Core の左ボタンでビープ音を On/Off　（上記事情により無効化）
- M5Core の中ボタンで SD カードにログ出力を開始/停止
  - csv形式で経過時間，イベント（後述），SpO2，HRを出力
  - ファイル名は日時のため繰り返し計測しても重複しない
  - 記録中は Rec アイコンを表示（下図 5）
  - 書込不可（SD カード未挿入）の場合には反応しない
- M5Core の右ボタンで ログ記録中のイベント（HR/SpO2 に影響しそうな事象（運動，呼吸停止，痰吸引など）の開始/終了）を記録
  - イベント発生中は Active アイコンを表示（下図 6）
- GUI でのエラー表示（Wi-Fi 未接続，センサ認識失敗，SD カード未挿入など，下図 1～3）
- バッテリ駆動により M5Core とセンサユニットだけで運用可
- コンパクトゆえに携帯/ウェアラブル機器として運用可

<img src="https://user-images.githubusercontent.com/25588291/113539152-0ad45080-9618-11eb-88d8-b4651447685b.jpg" width="600">

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
- [「M5Stack用心拍センサユニット」指クランプ化キット（STLデータ） - First DIY：まずは何か作ってみる - BOOTH](https://first-diy.booth.pm/items/2868138)（オプション）

# ToDo
- センサ使用とビープ音が干渉（現在はビープ音を無効化）

# Licence
[GPLv3](https://github.com/komix-DIYer/M5Fire_HRSpO2/blob/main/LICENSE)

# Author
komix
