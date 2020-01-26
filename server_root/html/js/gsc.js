'use strict';

(() => {
  const JQ_KEY = 'gsc line number'

  const DIGITS_RE = /[0-9]+/

  const extractNumber = (str) => {
    const match = DIGITS_RE.exec(str)
    if (match)
      return Number.parseInt(match[0])
  }

  const isVisible = (elt) => {
    return elt.position().top + elt.height() >= 0
  }

  window.GscFileViewer = class {
    constructor(viewerId) {
      const viewer   = $('#' + viewerId)
      const selector = viewer.find('.file-viewer-selector')
      const area     = viewer.find('.file-viewer-area')
      const lines    = area.find('td.code-line').get()
      const files    = area.find('div.single-file-viewer').get().map($)

      this.viewer    = viewer
      this.selector  = selector
      this.area      = area
      this.lines     = lines
      this.files     = files

      this.selector.change($.proxy(this.showSelectedFile, this))
      this.area.scroll($.proxy(this.selectShownFile, this))
      this.setupLineLinks()
    }

    showLine(lineNo) {
      this.showElement(this.line(lineNo), 0.3)
    }

    showFile(fileNo) {
      this.showElement(this.file(fileNo), 0)
    }

    file(index) {
      return this.files[index]
    }

    line(index) {
      const lines = this.lines
      const value = $(lines[index - 1])
      if (value.length) {
        lines[index - 1] = value
      }
      return value
    }

    setupLineLinks() {
      const lineCount = this.lines.length
      $('.line-link')
          .each((_, link) => {
            const number = extractNumber(link.innerText)
            $.data(link, JQ_KEY, number)
            if (number > lineCount) {
              link.className = 'broken-line-link'
            }
          })
          .click($.proxy(this.handleLineLink_, this))
    }

    handleLineLink_(evt) {
      this.showLine($.data(evt.target, JQ_KEY))
    }

    showSelectedFile() {
      this.showFile(this.selector.val())
    }

    selectShownFile() {
      this.selector.val(this.currentShownFile() || 0)
    }

    currentShownFile() {
      for (const file of this.files) {
        if (isVisible(file)) {
          return file.parent().index()
        }
      }
    }

    showElement(target, margin) {
      if (!target.length) {
        this.error()
        return
      }

      const area = this.area
      const goal = target.offset().top - margin * area.height()
      const delta = goal - area.offset().top
      area.scrollTop(area.scrollTop() + delta)

      target.effect('highlight', {color: 'purple'}, 500,)
    }

    error() {
      this.area.effect('shake', {distance: 10, times: 2}, 400)
    }
  }

  window.GSC = {
    // Select the given ID and copies it:
    selectCopyId: (id) => {
      const elt = document.getElementById(id)
      elt.select()
      document.execCommand('copy')
    },
  }
})()
