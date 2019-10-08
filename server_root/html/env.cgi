#!/bin/sh

html_encode () {
    sed '
        s|&|\&amp;|g
        s|<|\&lt;|g
        s|>|\&gt;|g
    '
}

echo content-type: text/html
echo
cat <<EOF
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>env | sort</title>
    <style>
      body {
        font-family: sans-serif;
        font-size: 85%;
      }

      h1 {
        font-family: monospace;
      }

      .env {
        width: 90%;
        margin: 1em auto;
        border-collapse: collapse;
      }

      th, td {
        vertical-align: top;
        text-align: left;
        padding: 4pt;
      }


      thead {
        color: white;
        background-color: black;
      }

      .value {
        max-width: 35em;
        overflow: scroll;
        white-space: pre-wrap;
        font-family: monospace;
      }
    </style>
  </head>
  <body>
    <h1>env | sort</h1>
    <table class="env">
      <thead>
        <tr>
          <th>Variable
          <th>Value
      <tbody>
        $(env | sort | html_encode | sed '
          /=/!d
          s|^|<tr><th>|
          s|=|<td class="value">|
        ' | tr -d '\n')
    </table>
  </body>
</html>
EOF
