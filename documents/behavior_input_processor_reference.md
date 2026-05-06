# ZMK Behavior / Input Processor Reference

この文書は `zmk-driver-iqs9151` が追加している ZMK 拡張のうち、`Behavior` と `Input Processor` を一覧化したものです。

`IQS9151` ドライバ本体については別文書で扱うため、ここには含めません。

## 1. 一覧

### Behaviors

|名前|compatible|役割|
| - | - | - |
|`trackpad-to-pos`|`zmk,behavior-trackpad-to-pos`|トラックパッド由来のイベントを仮想キー位置の press/release に変換する|
|`zip-dynamic-scale`|`zmk,behavior-zip-dynamic-scale`|動的スケーラの倍率を増減・リセットする|
|`zip-dynamic-scale-set`|`zmk,behavior-zip-dynamic-scale-set`|動的スケーラの倍率を指定値へ直接設定する|

### Input Processors

|名前|compatible|役割|
| - | - | - |
|`behaviors-consume`|`zmk,input-processor-behaviors-consume`|一致した入力イベントで Behavior を呼び出し、その入力イベント自体は無効化する|
|`dynamic-scaler`|`zmk,input-processor-dynamic-scaler`|`REL` 系イベントへ保持中のスケール倍率を適用する|

## 2. 全体像

このモジュール群は、主に以下の流れで組み合わせて使います。

1. IQS9151 ドライバが `INPUT_BTN_*` や `INPUT_REL_*` を出力する
2. `behaviors-consume` が特定の `BTN` イベントを Behavior に変換する
3. `trackpad-to-pos` が仮想キー位置の press/release を発火する
4. ZMK keymap 側でその仮想キー位置に任意のキーやマウス操作を割り当てる
5. `dynamic-scaler` がカーソル移動やスクロールの `REL` 値に倍率を掛ける
6. `zip-dynamic-scale` / `zip-dynamic-scale-set` がその倍率を実行中に変更する

要するに、

- `Behavior` は「イベントを ZMK のキー操作へ橋渡しする層」
- `Input Processor` は「入力イベントを途中で加工する層」

です。

## 3. Behaviors

### 3.1 `zmk,behavior-trackpad-to-pos`

トラックパッドのジェスチャやボタン相当イベントを、ZMK の仮想キー位置イベントへ変換する Behavior です。

- 実装: `behaviors/behavior_trackpad_to_pos.c`
- Binding: `dts/bindings/behavior/zmk,behavior-trackpad-to-pos.yaml`
- Kconfig: `CONFIG_ZMK_BEHAVIOR_TRACKPAD_TO_POS`
- `#binding-cells = <1>`
- 引数:
  - `param1`: 発火させたい仮想キー位置

### 使いどころ

- トラックパッドのタップやスワイプを、通常キーと同じように keymap に流したいとき
- `INPUT_BTN_*` をそのままマウスクリックとして出すのではなく、レイヤーやキーバインドに接続したいとき

### 定義例

```dts
/ {
    behaviors {
        tp_to_pos: tp_to_pos {
            compatible = "zmk,behavior-trackpad-to-pos";
            #binding-cells = <1>;
        };
    };
};
```

### 利用イメージ

```dts
bindings = <&tp_to_pos 52>;
```

上の例では、対象イベントが発生したときに「キー位置 `52` が押された / 離された」として ZMK に通知されます。

### 3.2 `zmk,behavior-zip-dynamic-scale`

`dynamic-scaler` input processor が使う倍率を、キー操作から増減またはリセットする Behavior です。

- 実装: `behaviors/behavior_zip_dynamic_scale.c`
- Binding: `dts/bindings/behavior/zmk,behavior-zip-dynamic-scale.yaml`
- Kconfig: `CONFIG_ZMK_BEHAVIOR_ZIP_DYNAMIC_SCALE`
- `#binding-cells = <2>`
- 引数:
  - `param1`: 対象グループ
  - `param2`: 操作種別

### グループ定数

`include/dt-bindings/zmk/zip_dynamic_scale.h`:

|定数|意味|
| - | - |
|`ZDS_XY`|カーソル移動用グループ|
|`ZDS_SC`|スクロール用グループ|
|`ZDS_ALL`|両方同時|

### 操作定数

|定数|意味|
| - | - |
|`ZDS_INC`|1段階上げる|
|`ZDS_DEC`|1段階下げる|
|`ZDS_RST`|デフォルト値へ戻す|

### 定義例

```dts
/ {
    behaviors {
        zip_dyn_scale: zip_dyn_scale {
            compatible = "zmk,behavior-zip-dynamic-scale";
            #binding-cells = <2>;
        };
    };
};
```

### keymap 例

```dts
&zip_dyn_scale ZDS_XY ZDS_INC
&zip_dyn_scale ZDS_XY ZDS_DEC
&zip_dyn_scale ZDS_SC ZDS_INC
&zip_dyn_scale ZDS_SC ZDS_DEC
&zip_dyn_scale ZDS_ALL ZDS_RST
```

### 3.3 `zmk,behavior-zip-dynamic-scale-set`

`dynamic-scaler` input processor が使う倍率を、指定値へ直接設定する Behavior です。

- 実装: `behaviors/behavior_zip_dynamic_scale_set.c`
- Binding: `dts/bindings/behavior/zmk,behavior-zip-dynamic-scale-set.yaml`
- Kconfig: `CONFIG_ZMK_BEHAVIOR_ZIP_DYNAMIC_SCALE_SET`
- `#binding-cells = <2>`
- 引数:
  - `param1`: 対象グループ
  - `param2`: 設定値 (`x10` スケール)

### 値の見方

- `10` = `1.0x`
- `5` = `0.5x`
- `15` = `1.5x`
- `20` = `2.0x`

### 定義例

```dts
/ {
    behaviors {
        zip_dyn_scale_set: zip_dyn_scale_set {
            compatible = "zmk,behavior-zip-dynamic-scale-set";
            #binding-cells = <2>;
        };
    };
};
```

### keymap 例

```dts
&zip_dyn_scale_set ZDS_XY 8
&zip_dyn_scale_set ZDS_SC 15
&zip_dyn_scale_set ZDS_ALL 10
```

## 4. Input Processors

### 4.1 `zmk,input-processor-behaviors-consume`

指定した入力コードに一致したとき、対応する Behavior を呼び出し、元のイベントは中和してマウスボタン出力などがそのまま流れないようにする Input Processor です。

- 実装: `input_processors/input_processor_behaviors_consume.c`
- Binding: `dts/bindings/input_processors/zmk,input-processor-behaviors-consume.yaml`
- Kconfig: `CONFIG_ZMK_INPUT_PROCESSOR_BEHAVIORS_CONSUME`
- `#input-processor-cells = <0>`

### 主なプロパティ

|プロパティ|型|内容|
| - | - | - |
|`type`|int|対象イベント種別。通常は `INPUT_EV_KEY`|
|`codes`|array|一致対象の入力コード一覧|
|`bindings`|phandle-array|各コードに対応して呼ぶ Behavior 一覧|

`codes` と `bindings` は同じ数である必要があります。

### 定義例

```dts
trackpad_key_behaviors_R: trackpad_key_behaviors_R {
    compatible = "zmk,input-processor-behaviors-consume";
    #input-processor-cells = <0>;
    type = <INPUT_EV_KEY>;
    codes = <INPUT_BTN_0 INPUT_BTN_1 INPUT_BTN_2>;
    bindings = <
        &tp_to_pos 55
        &tp_to_pos 56
        &tp_to_pos 57
    >;
};
```

### 向いている用途

- 1本指タップを左クリックに固定せず、任意の仮想キーへ変換したい
- 3本指スワイプを `MB4/MB5` 直出しではなく、レイヤー依存の挙動へつなげたい
- トラックパッド入力を keymap / Keymap Editor / Studio と連携しやすい形へ変換したい

### 本家 `zmk,input-processor-behaviors` との差分と注意点

ZMK 本体には、よく似た `zmk,input-processor-behaviors` があります。  
主な違いは、マッチ時の扱いです。

- 本家 `behaviors`:
  - Behavior を呼んだあと `ZMK_INPUT_PROC_STOP` を返して処理停止を狙う
  - 元イベント自体は書き換えない
- このモジュールの `behaviors-consume`:
  - Behavior を呼んだあと、元イベントを無害化して `CONTINUE` する
  - マウスボタン出力などが後段へそのまま流れにくい

特に `layer override` 内では、本家 `behaviors` の `STOP` が listener 側でそのまま尊重されず、元の `INPUT_BTN_*` が後続処理まで流れてしまう場合があります。  
その結果、置き換え先 Behavior と元のマウスボタンイベントが両方発火することがあります。

`behaviors-consume` はこの問題を避けるため、停止ではなくイベント中和で抑制する前提の実装になっています。

### 4.2 `zmk,input-processor-dynamic-scaler`

一致した `REL` イベントへ動的スケール倍率を掛ける Input Processor です。倍率は settings に保存され、電源再投入後も保持されます。

- 実装: `input_processors/input_processor_dynamic_scaler.c`
- Binding: `dts/bindings/input_processors/zmk,input-processor-dynamic-scaler.yaml`
- Kconfig: `CONFIG_ZMK_INPUT_PROCESSOR_DYNAMIC_SCALER`
- `#input-processor-cells = <0>`

### 主なプロパティ

|プロパティ|型|内容|
| - | - | - |
|`type`|int|対象イベント種別。通常は `INPUT_EV_REL`|
|`codes`|array|倍率を掛ける入力コード一覧|
|`scale-group`|int|保持する倍率グループ。`0: XY`, `1: Scroll`|

### 関連 Kconfig

|Symbol|Default|内容|
| - | - | - |
|`CONFIG_ZMK_INPUT_PROCESSOR_DYNAMIC_SCALER_DEFAULT_SCALE_X10`|`10`|初期倍率 (`1.0x`)|
|`CONFIG_ZMK_INPUT_PROCESSOR_DYNAMIC_SCALER_MIN_SCALE_X10`|`2`|最小倍率 (`0.2x`)|
|`CONFIG_ZMK_INPUT_PROCESSOR_DYNAMIC_SCALER_MAX_SCALE_X10`|`50`|最大倍率 (`5.0x`)|

### 定義例

```dts
zip_dynamic_xy_scaler: zip_dynamic_xy_scaler {
    compatible = "zmk,input-processor-dynamic-scaler";
    #input-processor-cells = <0>;
    type = <INPUT_EV_REL>;
    codes = <INPUT_REL_X INPUT_REL_Y>;
    scale-group = <ZDS_XY>;
};

zip_dynamic_scroll_scaler: zip_dynamic_scroll_scaler {
    compatible = "zmk,input-processor-dynamic-scaler";
    #input-processor-cells = <0>;
    type = <INPUT_EV_REL>;
    codes = <INPUT_REL_WHEEL INPUT_REL_HWHEEL>;
    scale-group = <ZDS_SC>;
};
```

### 向いている用途

- キー操作でトラックパッド速度を即座に切り替えたい
- カーソル速度とスクロール速度を別々に調整したい
- 設定を再起動後も保持したい

### 4.3 `zmk,input-processor-iqs9151-split-inertia-filter`

split 構成で peripheral 側 IQS9151 から届くスクロールイベントのうち、ドライバが付与した「慣性スクロール中」タグ付きイベントだけを識別して抑止する Input Processor です。

- 実装: `input_processors/input_processor_iqs9151_split_inertia_filter.c`
- Binding: `dts/bindings/input_processors/zmk,input-processor-iqs9151-split-inertia-filter.yaml`
- Kconfig: `CONFIG_ZMK_INPUT_PROCESSOR_IQS9151_SPLIT_INERTIA_FILTER`
- `#input-processor-cells = <0>`

### 主な役割

- peripheral 側で生成された慣性スクロールイベントだけを通常スクロールと区別する
- central 側で修飾キー押下による慣性キャンセル要求が出ている短時間だけ、タグ付きイベントを中和する
- 手動スクロールが始まったら、その listener インスタンスの suppress 状態だけを解除する

### 定義例

```dts
#include <input/processors/iqs9151_split_inertia_filter.dtsi>

/ {
    trackpad_listener {
        compatible = "zmk,input-listener";
        device = <&trackpad_split_R>;
        input-processors = <
            &iqs9151_split_inertia_filter
            &zip_dynamic_scroll_scaler
        >;
    };
};
```

### 向いている用途

- split キーボードで peripheral 側トラックパッドの慣性スクロールを、central 側の modifier 押下で止めたい
- 通常の手動スクロールは通しつつ、慣性継続分だけを抑止したい

### 注意点

- `CONFIG_INPUT_IQS9151_SCROLL_INERTIA_CANCEL_ON_MODIFIERS=y` を併用してください。
- `input-processors` の先頭に置いて、タグ付き慣性イベントを後段へ渡す前に復号・抑止してください。
- central 側で split listener を複数使う場合も、各 listener ごとに suppress 状態を独立して保持します。

## 5. 組み合わせ例

`LaLaPad Gen2` 相当の構成を簡略化すると以下のようになります。

```dts
/ {
    behaviors {
        /* behavior-trackpad-to-posの定義 */
        tp_to_pos: tp_to_pos {
            compatible = "zmk,behavior-trackpad-to-pos";
            #binding-cells = <1>;
        };

        /* behavior-zip-dynamic-scaleの定義 */
        zip_dyn_scale: zip_dyn_scale {
            compatible = "zmk,behavior-zip-dynamic-scale";
            #binding-cells = <2>;
        };
    };

    /* BTNイベントを仮想キーへ流し直す */
    trackpad_key_behaviors {
        compatible = "zmk,input-processor-behaviors-consume";
        #input-processor-cells = <0>;
        type = <INPUT_EV_KEY>;
        codes = <INPUT_BTN_0 INPUT_BTN_1 INPUT_BTN_2>;
        bindings = <
            &tp_to_pos 52
            &tp_to_pos 53
            &tp_to_pos 54
        >;
    };

    /* カーソル移動のREL値に倍率を掛ける */
    zip_dynamic_xy_scaler {
        compatible = "zmk,input-processor-dynamic-scaler";
        #input-processor-cells = <0>;
        type = <INPUT_EV_REL>;
        codes = <INPUT_REL_X INPUT_REL_Y>;
        scale-group = <ZDS_XY>;
    };

    /* listenerで各 processor を処理順に接続する */
    trackpad_listener {
        compatible = "zmk,input-listener";
        device = <&iqs9151>;
        input-processors = <&trackpad_key_behaviors>,
                           <&zip_dynamic_xy_scaler>;
    };
};
```

これにより、

- `INPUT_BTN_*` は仮想キーへ変換される
- `INPUT_REL_X/Y` は現在倍率で拡大または縮小される
- keymap から倍率を変更できる

という構成になります。

## 6. 補足

- `zip_dynamic_scale` 系 Behavior は、単体では意味を持たず `dynamic-scaler` と組み合わせて使います。
- `behaviors-consume` は「ボタンを直接マウスイベントとして出す」用途ではなく、「Behavior に流し直す」用途向けです。
- `input-processors` の並び順は処理順になります。`behaviors-consume` とスケーラ系を混在させる場合は、どのイベントをどの段階で加工したいかを意識して並べてください。
- `zip_xy_scaler` / `zip_scroll_scaler` は ZMK 標準側の processor 参照であり、このリポジトリで追加しているものではありません。
