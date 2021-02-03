# HTML content parsing
## Tokens
The HTML content is parsed into an array of tokens and since the data that is
received from the API always has 42 characters between `\n`, line-wrapping in
ncurses will automatically handle new rows. Each token which will have all the
attributes needed for the UI to render it properly.

Rendering of the tokens is done by iterating through each token until
`PAGE_TOKEN_END` is found. `PAGE_TOKEN_NEWLINE` marks the end of the current
line. However, new lines does not have to be handled manually, since the total
character length of all previous tokens (after the previous `PAGE_TOKEN_NEWLINE`
or start of the array) will always be 42 characters (as defined by `PAGE_COLS`).

### Types
There are 5 different token types:
```c
typedef enum page_token_type {
    PAGE_TOKEN_TEXT,
    PAGE_TOKEN_WHITESPACE,
    PAGE_TOKEN_LINK,
    PAGE_TOKEN_NEWLINE,
    PAGE_TOKEN_END
} page_token_type_t;
```

### Styling
Each token will have a style, as defined by the struct:
```c
typedef struct page_token_style page_token_style_t;

struct page_token_style {
    page_token_attr_t fg;
    page_token_attr_t bg;
    page_token_attr_t extra; // other attributes, e.g. bold text
};
```

The available attributes for the style are the following:
```c
typedef enum page_token_attr {
    PAGE_TOKEN_ATTR_BOLD,       // .DH
    PAGE_TOKEN_ATTR_BLUE,       // .B
    PAGE_TOKEN_ATTR_CYAN,       // .C
    PAGE_TOKEN_ATTR_WHITE,      // .W
    PAGE_TOKEN_ATTR_GREEN,      // .G
    PAGE_TOKEN_ATTR_YELLOW,     // .Y
    PAGE_TOKEN_ATTR_RED,        // .R
    PAGE_TOKEN_ATTR_BG_BLUE,    // .bgB
    PAGE_TOKEN_ATTR_BG_CYAN,    // .bgC
    PAGE_TOKEN_ATTR_BG_WHITE,   // .bgW
    PAGE_TOKEN_ATTR_BG_GREEN,   // .bgG
    PAGE_TOKEN_ATTR_BG_YELLOW,  // .bgY
    PAGE_TOKEN_ATTR_BG_RED,     // .bgR
} page_token_attr_t;
```

### Colors
This CSS-code shows all the possible classes and the styling that is used
(extracted from [texttv.nu](https://texttv.nu/)):

```css
.TextTVPage
a {
	color: rgb(221, 221, 221)
}

.TextTVPage
.root {
	white-space: pre
}

.TextTVPage .root
.toprow {
	color: #ddd;
	line-height: 1
}

.TextTVPage .inpage-pages {
	list-style-type: none;
	margin: 0;
	padding: 0
}

.TextTVPage h1,
.TextTVPage
h2 {
	font-size: 100%;
	display: inline
}

.TextTVPage
.B {
	color: #336
}

.TextTVPage
.C {
	color: #066
}

.TextTVPage
.W {
	color: #333
}

.TextTVPage
.Y {
	color: #660
}

.TextTVPage
.DH {
	font-weight: normal;
	font-family: 'Ubuntu Mono',Courier;
	font-weight: 700
}

.TextTVPage .bgB,
.TextTVPage .bgB a
span {
}

.TextTVPage .bgW,
.TextTVPage .bgW a
span {
	background-color: orange
}

.TextTVPage .bgG,
.TextTVPage .bgG a
span {
	background-color: rgba(0,255,0,.5)
}

.TextTVPage .bgR,
.TextTVPage .bgR a
span {
	background-color: rgba(255,0,0,.25)
}

.TextTVPage .bgC,
.TextTVPage .bgC a
span {
	background-color: rgb(0, 200, 238)
}

.TextTVPage .R,
.TextTVPage .R a,
.TextTVPage .R a:visited,
.TextTVPage .R a:visited:hover {
	color: #F00
}

.TextTVPage .B,
.TextTVPage .B
a {
	color: #12C
}

.TextTVPage .W,
.TextTVPage .W
a {
	color: #ddd
}

.TextTVPage .Y,
.TextTVPage .Y
a {
	color: #e2e200
}

.TextTVPage .C,
.TextTVPage .C
a {
	color: #00c8ee
}

.TextTVPage .bgB,
.TextTVPage .bgB
a {
	background-color: #00f;
	color: #ddd
}

.TextTVPage
.DH {
}

.TextTVPage .G,
.TextTVPage .G
a {
	color: #0F0
}

.TextTVPage .bgY,
.TextTVPage .bgY
a {
	background-color: #e2e200;
	color: #12C
}
```
