data = read.csv(file = 'trained_data.csv')
# data = data[data$Chunk.Size == "4",]
# data$Type = as.factor(data$Type)
# data = data[order(as.numeric(data$Type)),]
model <- lm(Billed.Duration.ms. ~ Read.Size.MB. + Returned.Size.MB. +
Transferred.Size.MB., data = data)

summary(model) 