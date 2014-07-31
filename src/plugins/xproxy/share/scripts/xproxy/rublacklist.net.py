from bs4 import BeautifulSoup
from PyQt4.QtCore import QUrl, QStringList
from PyQt4.QtNetwork import QNetworkAccessManager, QNetworkRequest

def getListName():
	return "RuBlackList.net"

nam = QNetworkAccessManager()

def handleFinished(req, elems = [], tries = 0):
	data = req.readAll()
	req.deleteLater()
	soup = BeautifulSoup(str(data))

	tbody = soup.select("body div div table tbody")
	if len(tbody) != 1:
		if tries < 3:
			nextReq = nam.get(QNetworkRequest(req.request().url()))
			nextReq.finished.connect(lambda: handleFinished(nextReq, elems, tries + 1))
		else:
			xproxy.reportError("Cannot parse markup")
			return

	tbody = tbody[0]

	for child in tbody.children:
		if child.name != "tr":
			continue
		link = child.find("a")
		if link is None:
			continue
		linkStr = link.string
		if linkStr.startswith("http://"):
			elems.append(unicode(linkStr))

	navStr = soup.find("div", class_ = "pagination").b.get_text()
	partition = navStr.partition("/")
	curIdxStr = partition[0]
	lastIdxStr = partition[2]
	if curIdxStr == lastIdxStr:
		xproxy.setUrls(elems)
		return

	nextIdx = int(curIdxStr) + 1

	nextReq = nam.get(QNetworkRequest(QUrl("http://reestr.rublacklist.net/pages/" + str(nextIdx))))
	nextReq.finished.connect(lambda: handleFinished(nextReq, elems))

def refresh():
	req = nam.get(QNetworkRequest(QUrl("http://reestr.rublacklist.net/")))
	req.finished.connect(lambda: handleFinished(req))
