'use strict';

export const GSC = {
  // Select the given ID and copies it:
  selectCopyId: (id) => {
    const elt = document.getElementById(id)
    elt.select()
    document.execCommand('copy')
  },
}

export function GscFileViewer(id) {
  const viewer    = $('#' + id)
  const selector  = viewer.find('.file-viewer-selector')
  const area      = viewer.find('.file-viewer-area')
  const lines     = area.find('td.code-line')
  const files     = area.find('div.single-file-viewer')

  this.viewer     = viewer
  this.selector   = selector
  this.area       = area
  this.lines      = lines
  this.files      = files

  this.setupLineLinks()

  area.scroll($.proxy(this.selectShownFile, this))
  selector.change($.proxy(this.showSelectedFile, this))
}

{
  const JQ_KEY = 'gsc line number'

  const DIGITS = /[0-9]+/

  const extractNumber = (str) => {
    const match = DIGITS.exec(str)
    if (match)
      return Number.parseInt(match[0])
  }

  const isVisible = (elt) => {
    return elt.position().top + elt.height() >= 0
  }

  const proto = GscFileViewer.prototype

  proto.setupLineLinks = function () {
    const lineCount = this.lines.length
    $('.line-link')
        .each((_, link) => {
          const number = extractNumber(link.innerText)
          $.data(link, JQ_KEY, number)
          if (number > lineCount) {
            link.className = 'broken-line-link'
          }
        })
        .click($.proxy(this.handleLineLink, this))
  }

  proto.handleLineLink = function (evt) {
    this.showLine($.data(evt.target, JQ_KEY))
  }

  proto.showSelectedFile = function () {
    this.showFile(this.selector.val())
  }

  proto.selectShownFile = function () {
    const index =
        this.files
            .filter((_, e) => isVisible($(e)))
            .first()
            .parent()
            .index()
    this.selector.val(index || 0)
  }

  proto.showLine = function (lineNo) {
    this.showElement(this.lines.eq(lineNo - 1), 0.3)
  }

  proto.showFile = function (fileNo) {
    this.showElement(this.files.eq(fileNo), 0)
  }

  proto.showElement = function (target, margin) {
    if (!target.length) {
      this.error()
      return
    }

    const area = this.area
    const goal = target.offset().top - margin * area.height()
    const delta = goal - area.offset().top
    area.scrollTop(area.scrollTop() + delta)

    target.effect('highlight', {color: 'white'}, 700,)
  }

  proto.error = function () {
    this.area.effect('shake', {distance: 10, times: 2}, 400)
  }
}
