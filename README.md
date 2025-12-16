# ForiverEngine
DirectX12 を用いてフルスクラッチで制作した、自作ゲームエンジン

## 進捗

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

## OSS
| 名前 | ライセンス |
| --- | --- |
| [SimplexNoise](https://github.com/SRombauts/SimplexNoise) | [MIT](https://github.com/SRombauts/SimplexNoise/blob/master/LICENSE.txt) |
| [DirectXTex](https://github.com/microsoft/DirectXTex) | [MIT](https://github.com/microsoft/DirectXTex/blob/main/LICENSE) |
