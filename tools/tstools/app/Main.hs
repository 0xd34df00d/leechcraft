{-# LANGUAGE OverloadedStrings, TemplateHaskell, QuasiQuotes #-}
{-# LANGUAGE DataKinds, TypeOperators, DeriveGeneric, FlexibleInstances, RecordWildCards #-}

module Main where

import qualified Control.Foldl as F
import qualified Data.List as L
import qualified Data.Text as T
import qualified Data.Text.IO as T
import Control.Arrow
import Control.Monad
import Data.FileEmbed
import Data.Functor
import Data.Maybe
import Data.String.Interpolate.IsString
import Options.Generic
import Prelude hiding(FilePath)
import Turtle

xsltStyle :: Text
xsltStyle = $(embedStringFile "transform.xsl")

mkGenerated :: MonadIO io => [FilePath] -> io [FilePath]
mkGenerated allFiles
  | null settingsFiles = pure []
  | otherwise = do
      liftIO $ runManaged $ do
        xsltFile <- mktempfile "." "xxxxxx.xslt"
        liftIO $ T.writeFile xsltFile xsltStyle
        output "dummy.cpp" $ cat $ procFile xsltFile <$> settingsFiles
      pure ["dummy.cpp"]
  where
    settingsFiles = mapMaybe settingsExtractor allFiles
    settingsExtractor str = first (const str) <$> L.find ((`T.isSuffixOf` T.pack str) . fst) [ ("settings.xml", basename)
                                                                                               , (".qml.settings", filename)
                                                                                               ]
    procFile xsltFile (settingFile, contextFun) = sed ("__FILENAME__" $> T.pack (contextFun settingFile))
                                                $ grep (has "QT_TRANSL")
                                                $ inproc "xsltproc" [T.pack xsltFile, T.pack settingFile] empty

guessTsBase :: FilePath -> FilePath
guessTsBase fullPath = go "leechcraft" $ tail $ dropWhile (/= "src") $ T.takeWhile (/= '/') . T.pack <$> splitDirectories fullPath
  where
    go acc [] = acc
    go acc ("plugins" : plugin : rest) = go [i|#{acc}_#{plugin}|] rest
    go _   comps = error [i|Unparseable components: #{comps}|]

data Options w = Options
  { path :: w ::: Maybe String <?> "Path to the plugin directory"
  , languages :: w ::: [String] <?> "List of languages to generate or update translations for (update all if empty)"
  , dropObsolete :: w ::: Bool <?> "Drop obsolete translations"
  } deriving (Generic)

instance ParseRecord (Options Wrapped)

extensionIs :: FilePath -> String -> Bool
extensionIs path ext = extension path == Just ext

find'lupdate :: MonadIO io => io (Maybe FilePath)
find'lupdate = do
  inPath <- which "lupdate"
  case inPath of
    Just path -> pure $ Just path
    Nothing -> listToMaybe <$> testfile `filterM` [ "/usr/lib/qt5/bin/lupdate", "/usr/lib64/qt5/bin/lupdate" ]

main :: IO ()
main = do
  Options { .. } <- unwrapRecord "tstools"

  let noobsoleteArg | dropObsolete = ["-noobsolete"]
                    | otherwise = []

  case path of
       Just path' -> cd $ fromString path'
       Nothing -> pure ()

  files <- lsif (\subpath -> pure $ basename subpath /= "plugins") "." `fold` F.list
  let sources = filter (\file -> any (file `extensionIs`) ["cpp", "ui", "qml"]) files
  generated <- mkGenerated files

  tsFiles <- case languages of
                  [] -> pure $ filter (`extensionIs` "ts") files
                  _ -> do
                        tsBase <- guessTsBase <$> pwd
                        pure $ (\lang -> [i|#{T.pack tsBase}_#{lang}.ts|]) <$> languages
  lupdate <- find'lupdate >>= maybe (fail "`lupdate` not found; is it in your path?") (pure . T.pack)
  forM_ tsFiles $ \tsFile -> do
    let lupdateArgs = noobsoleteArg <> fmap T.pack (sources <> generated) <> ["-ts", T.pack tsFile]
    view $ inproc lupdate lupdateArgs empty

  mapM_ rm generated
