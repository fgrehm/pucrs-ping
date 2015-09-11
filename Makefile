build:
	docker build -t fgrehm/pucrs-ping .

hack:
	docker run -ti --rm -v `pwd`:/workspace -w /workspace fgrehm/pucrs-ping bash
