# upload the image-upload function
gcloud functions deploy image-upload --entry-point MoodCap.PostImage --runtime dotnet3 --trigger-http --allow-unauthenticated
# upload the image-list function
gcloud functions deploy image-list --entry-point MoodCap.ListImages --runtime dotnet3 --trigger-http --allow-unauthenticated
# upload the images-upload function
gcloud functions deploy images-upload --entry-point MoodCap.PostImages --runtime dotnet3 --trigger-http --allow-unauthenticated


# for db Connection
psql "sslmode=disable dbname=moodcap user=postgres hostaddr=34.90.206.100"

# upload the data-upload function
### this function has been used for testing purpose only
# gcloud functions deploy data-upload --entry-point MoodCap.PostData --runtime dotnet3 --trigger-http --allow-unauthenticated
