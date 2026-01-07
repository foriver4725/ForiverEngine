# ForiverEngine

### > [Wiki](https://github.com/foriver4725/ForiverEngine/wiki)

## 過去の進捗

<details>

<summary> 内容 </summary>

画面を黄色にして表示する

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/46cca266-f217-414f-88f9-3a084312d3cf" />

---

コンソール画面も併せて表示する

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/81d99d04-f237-4255-9228-82dd80b09300" />

---

ポリゴンを描画する

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/6eedfb76-7f27-4da0-9b56-197bb5ad51d6" />

---

UV座標を渡す

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/60b61bae-83a6-4439-81b2-374f045079bb" />

---

テクスチャを渡す (テクスチャデータはプログラムで決め打ち)

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/bfa0e465-85cb-4828-9d52-9f9c18c529ed" />

---

プロジェクト内のアセットから、テクスチャをロードする

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/201cd605-82ea-45b9-8a49-beb81ecc9124" />

---

線形代数の算術を定義  
Transform構造体を作り、MVP行列を計算する

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/bbccd236-f7a4-46e3-9884-42c9d064e2a3" />

---

立方体の頂点情報やテクスチャを定義し、3D表示する  
ループ内でくるくる回す  
まだ深度バッファーを導入していないので、深度テストは行われていない

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/21295e88-391a-4c44-9e63-a064746e8d31" />

---

深度バッファーを導入

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/42e1c754-5cd8-4b02-b8b0-322c00fa6f91" />

---

2ブロック分のテクスチャを1枚にまとめ、効率的に使用する

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/7927f9cc-2ef5-4d64-b104-e9f7145015ba" />

---

フレームループを定義し、入力フラグを管理する

https://github.com/user-attachments/assets/997b0030-4f6a-4257-81c2-c25ae09285a7

---

複数テクスチャをテクスチャ配列としてロードし、動的に切り替え可能にする

https://github.com/user-attachments/assets/c85485b3-ea79-44a8-9014-86f93b563cf1

---

ターゲットFPSに合わせて、フレームループを回す

<img width="30%" alt="image" src="https://github.com/user-attachments/assets/1cd4f815-8151-4b2e-8363-4671e94c1977" />

---

地形データを作成し、表面に出ているフェースのみをメッシュ結合して描画する  
また、入力を受け取ってカメラを動かす  

https://github.com/user-attachments/assets/df66a750-5a09-4159-a8b8-933a0f391615

---

基本的な3D操作を実装する  
地形のデータ外に出ると、エラーが出る

https://github.com/user-attachments/assets/2dadb94c-5942-4264-a9eb-628961f666d0

---

シンプルノイズを用いて、地形を生成する

https://github.com/user-attachments/assets/b112ab78-a5db-4814-bee5-90f647d4079a

---

3D空間内を、「まともに」動けるようにする

https://github.com/user-attachments/assets/109e7838-61ae-4900-9f60-24a9c9b0964c

---

地形をチャンクに区切って、たくさん描画する

https://github.com/user-attachments/assets/9626a7e3-f360-4ce9-975c-0f16392d2aff

---

向いているブロックの色を変える

https://github.com/user-attachments/assets/944b3c8a-25db-41ae-80a7-41a8348059d6

---

ブロックを壊す

https://github.com/user-attachments/assets/a7f54352-877b-468c-a954-d8399b68ff76

---

簡易ライティング (正規化ランバートと環境光) を実装

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/245b3668-1689-4797-8391-5f55ba24376e" />

---

描画距離を設定し、その範囲内の地形についてだけ、ドローコールを発行する

https://github.com/user-attachments/assets/6990580c-cd22-4d50-ab5d-8f627ca79271

---

地中の当たり判定を実装

https://github.com/user-attachments/assets/c2fc2e4c-d963-4b22-aefe-aff62fdff570

---

ポストプロセスを実装

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/a080e8ec-9413-4d3e-8e3b-09c30b68dcf8" />

---

アンチエイリアスを実装 (FXAA っぽい何か)

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/84aa542a-3b84-41b4-b725-baec4ab001c0" />

---

テキストを画面に描画する (初期化以降の変更不可)

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/b3974110-1ddd-4aed-8121-5fdd90a6cb49" />

---

画面に出るテキストを、いつでも変更できるようにした

https://github.com/user-attachments/assets/966930bb-87e3-4a83-8b32-b89a23176837

---

描画チャンク更新時に、未生成のチャンクがあるならば新規生成するようにした  
地形データとメッシュの作成は並列処理で行い、頂点・インデックスバッファビューの作成はメインスレッドで1フレームで処理している  
これによって、地形の動的無限生成が理論上可能になり、起動時のロード時間が大幅に短縮された

https://github.com/user-attachments/assets/6cca3cdb-92a6-4fc9-ad82-9b307a505bd3

---

- プレイヤーのコリジョン計算を 2x2 のチャンク群で行うようにし、プレイヤーがチャンクを跨いでいる時の判定精度を高めた
- コリジョン計算などのテストコードを作成
- 画面に出すデバッグ情報を追加

<img width="50%" alt="image" src="https://github.com/user-attachments/assets/de49d60e-2563-440c-bb1d-a0b2942b7cdd" />

</details>

## 最新の進捗
ブロック設置を出来るようにした  
Q で採掘、E で石を設置

https://github.com/user-attachments/assets/22c88a88-05af-4890-b2c8-a31769bd332b

## 使用ライブラリ
| 名前 | ライセンス | 主な使用用途 |
| --- | --- | --- |
| [SimplexNoise](https://github.com/SRombauts/SimplexNoise) | [MIT](https://github.com/SRombauts/SimplexNoise/blob/master/LICENSE.txt) | ノイズを使った地形生成 |
| [DirectXTex](https://github.com/microsoft/DirectXTex) | [MIT](https://github.com/microsoft/DirectXTex/blob/main/LICENSE) | 画像をテクスチャとしてロードする |
